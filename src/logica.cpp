//
// Created by sait1 on 02/05/2026.
//

#include "logica.h"
#include "sensores.h"
#include "motor_servos.h"
#include "comunicaciones.h"
#include "diagnostico.h"
#include "GLOBALES.h"

void moverBrazo() {
    bool valido = false;

    if (peso >= peso_chica-umbral_peso && peso <= peso_chica+umbral_peso && tamanio >= largo_chicas-umbral_largo && tamanio <= largo_chicas+umbral_largo) {
        controlarBrazo(PRIMER_BRAZO);
        contador_chicas++;
        categoria_actual = "CHICA";
        valido = true;
        color_punto_led(7, tira.Color(2, 247, 223));
    }
    else if (peso >= peso_medianas-umbral_peso && peso <= peso_medianas+umbral_peso && tamanio >= largo_medianas-umbral_largo && tamanio <= largo_medianas+umbral_largo) {
        controlarBrazo(SEGUNDO_BRAZO);
        contador_medianas++;
        categoria_actual = "MEDIANA";
        valido = true;
        color_punto_led(7, tira.Color(247, 2, 243));
    }
    else if (peso >= peso_grandes-umbral_peso && peso <= peso_grandes+umbral_peso && tamanio >= largo_grande-umbral_largo && tamanio <= largo_grande+umbral_largo) {
        controlarBrazo(TERCER_BRAZO);
        contador_grandes++;
        categoria_actual = "GRANDE";
        valido = true;
        color_punto_led(7, tira.Color(247, 186, 2));
    }

    if (!valido) {
        categoria_actual = "NO VALIDAS";
        no_validas++;
        color_punto_led(7, tira.Color(255,0,0));
        sonar_alarma(1,100);
    }
}

void procesar() {
    distancia_lector = usar_laser();
    float grosor_actual = altitud_cinta - distancia_lector;
    float pes = obtenerPeso();

    switch (estado_proceso) {

        // ─── ESTADO 0: Esperando galleta en báscula ───
        case ESPERA:
            if (pes > umbral_peso) {
                pesos = pes;
                tiempo_entrada = millis();
                grosor_maximo = 0;
                motor_B(ADELANTE);           // Empieza a subir la base
                estado_proceso = SUBIENDO;
                Serial.println("[ESTADO] Galleta detectada → Subiendo");
            }
            break;

        // ─── ESTADO 1: Motor B subiendo ───
        case SUBIENDO:
            if (millis() - tiempo_entrada >= 2000) {  // Ajusta este tiempo
                motor_B(ATRAS);                        // Empieza a bajar
                tiempo_entrada = millis();             // Reinicia temporizador
                estado_proceso = BAJANDO;
                Serial.println("[ESTADO] Subida completa → Bajando");
            }
            break;

        // ─── ESTADO 2: Motor B bajando (galleta se resbala a la cinta) ───
        case BAJANDO:
            if (millis() - tiempo_entrada >= 2000) {  // Ajusta este tiempo
                motor_B(PARAR);
                tiempo_entrada = millis();             // Reinicia para medir largo
                objeto = true;
                estado_proceso = MIDIENDO;
                Serial.println("[ESTADO] Base abajo → Midiendo en cinta");
            }
            break;

        // ─── ESTADO 3: Midiendo grosor y largo en la cinta ───
        case MIDIENDO:
            if (objeto && grosor_actual > grosor_maximo) {
                grosor_maximo = grosor_actual;
            }

            // Galleta terminó de pasar por el láser
            if (objeto && grosor_actual < umbral_largo) {
                tiempo_en_pasar = millis() - tiempo_entrada;
                tamanio = tiempo_en_pasar * velocidad_de_cinta;
                peso = pesos;
                objeto = false;
                estado_proceso = CLASIFICANDO;
                Serial.println("[ESTADO] Medición completa → Clasificando");
            }
            break;

        // ─── ESTADO 4: Clasificar y reiniciar ───
        case CLASIFICANDO:
            moverBrazo();
            obtenerValores();
            // enviar_a_base_de_datos();
            estado_proceso = ESPERA;
            Serial.println("[ESTADO] Clasificado → Esperando siguiente galleta");
            break;
    }
}