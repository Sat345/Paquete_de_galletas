//
// Created by sait1 on 02/05/2026.
//

#include "diagnostico.h"
#include "GLOBALES.h"
#include "motor_servos.h"
#include "sensores.h"
#include "comunicaciones.h"

Adafruit_NeoPixel tira = Adafruit_NeoPixel(NUM_LEDS, PIN_LEDS, NEO_GRB + NEO_KHZ800);


void registrar_error(CodigoError codigo, String mensaje) {
    Serial.print("[LOG BANCO]: ");
    Serial.println(mensaje);

    ultimo_error_codigo = codigo;

    if (codigo != SIN_ERROR) {
        enviar_error_a_base_de_datos(codigo, mensaje);
    }
}


void medir_5V() {
    // 1. Leer el voltaje en el pin (0 a 3.3V)
    int lectura_raw = analogRead(34);
    float v_pin = (lectura_raw * 3.3) / 4095.0;

    // 2. CÁLCULO DE VOLTAJE TOTAL (V)
    // R1=10k y R2=10k, el voltaje de la fuente es el doble
    float v_fuente = v_pin * 2.0;

    // 3. CÁLCULO DE CORRIENTE (I) que pasa por el divisor
    // I = V / R. Usamos la resistencia de abajo (10,000 ohms)
    // Corriente = Voltaje del pin / 10,000
    float corriente_amperios = v_pin / 10000.0;
    float corriente_miliamperios = corriente_amperios * 1000.0;

    // 4. CÁLCULO DE RESISTENCIA TOTAL (R) del divisor
    // R = V / I.
    float r_total = 0;
    if (corriente_amperios > 0) {
        r_total = v_fuente / corriente_amperios;
    }

    // Guardar en tus variables globales
    voltaje_5V = v_fuente;
    corriente_5V = corriente_miliamperios;
    resistencia_5V = r_total;
}

// En tu archivo de diagnóstico
void medir_12V() {
    int lectura_12v = analogRead(35);
    float v_pin35 = (lectura_12v * 3.3) / 4095.0;

    // 33k y 10k,
    // V_fuente = V_pin * ((R1 + R2) / R2) -> V_pin * 4.3
    float voltaje_12v_real = v_pin35 * 4.511;

    float corriente_amperios = v_pin35 / 9400.0;
    float corriente_miliamperios = corriente_amperios * 1000.0;

    // 4. CÁLCULO DE RESISTENCIA TOTAL (R) del divisor
    // R = V / I.
    float r_total = 0;
    if (corriente_amperios > 0) {
        r_total = voltaje_12v_real / corriente_amperios;
    }

    voltaje_12V = voltaje_12v_real;
    corriente_12V = corriente_miliamperios;
    resistencia_12V = r_total;
}

void verificarSaludEnergia() {
    medir_5V();
    medir_12V();
    bool aceptable = true;

    if (voltaje_5V < 4.5) {

        aceptable = false;
        registrar_error(ERROR_BAJO_VOLTAJE_5V,"¡CRÍTICO: Voltaje bajo! \n Voltaje de 5V:" + String(voltaje_5V));
    }
    if (voltaje_5V >5.3){

        aceptable = false;
        registrar_error(ERROR_ALTO_VOLTAJE_5V,"¡CRÍTICO: Voltaje alto! \n Voltaje de 5V:" + String(voltaje_5V));
    }
    if (corriente_5V < 0.1) {

        aceptable = false;
        registrar_error(ERROR_BAJA_CORRIENTE_5V,"ERROR: Sensor de voltaje desconectado \n Corriente de 5V:" + String(corriente_5V));
    }
    if (corriente_5V > 1) {

        aceptable = false;
        registrar_error(ERROR_ALTA_CORRIENTE_5V,"ERROR: Error de resistencias o posible corto \n Corriente de 5V:" + String(corriente_5V));
    }
    if (voltaje_12V < 10.5) {
        aceptable = false;
        registrar_error(ERROR_BAJO_VOLTAJE_12V,"¡ALERTA! Fuente de 12V inestable \n Voltaje de 12V:" +  String(voltaje_12V));
    }
    if (voltaje_12V > 13.5) {
        aceptable = false;
        registrar_error(ERROR_ALTO_VOLTAJE_12V,"¡ALERTA CRÍTICA! SOBREVOLTAJE EN LÍNEA DE 12V \n Voltaje de 12V"+ String(voltaje_12V));
    }
    if (resistencia_12V < 35000.0 ) {
        aceptable = false;
        registrar_error(ERROR_BAJA_RESISTENCIA_12V,"¡ALERTA CRÍTICA! Resistencia del divisor muy baja \n Resistencia de 12V:"+ String(resistencia_12V));
    }
    if ( resistencia_12V > 50000.0) {
        aceptable = false;
        registrar_error(ERROR_ALTA_RESISTENCIA_12V,"¡ALERTA CRÍTICA! Resistencia del divisor muy alta \n Resistencia de 12V:"+ String(resistencia_12V));
    }
    if ( corriente_12V > 0.40) {
        aceptable = false;
        registrar_error(ERROR_ALTA_CORRIENTE_12V,"¡ALERTA CRÍTICA! Resistencia del divisor muy alta \n Corriente de 12V:"+ String(corriente_12V));
    }
    if (sistemaActivo && corriente_12V < 0.10) {
        aceptable = false;
        registrar_error(ERROR_BAJA_CORRIENTE_12V,"¡ALERTA CRÍTICA! Resistencia del divisor muy baja \n Corriente de 12V:" + String(corriente_12V));
    }


    if (aceptable){
        color_punto_led(4, tira.Color(0, 0, 255));
    }else {
        controlarCinta(PARAR);
        sistemaActivo = false;
        alarma_critica(255, 255, 0);
        color_punto_led(4, tira.Color(255, 0, 0));
    }
}

void medir_corriente_motor() {
    long suma_lecturas = 0;
    int muestras_validas = 0;
    int total_intentos = 100; // Incrementamos muestras para promediar el ruido del L298N

    for (int i = 0; i < total_intentos; i++) {
        int lectura = analogRead(32);

        if (lectura < 600) {
            suma_lecturas += lectura;
            muestras_validas++;
        }
        delayMicroseconds(30);
    }

    if (muestras_validas == 0) muestras_validas = 1;

    float lectura_promedio = (float)suma_lecturas / muestras_validas;
    float v_pin32 = (lectura_promedio * 1.1) / 4095.0;

    // Ley de Ohm: I = V / R
    float corriente_motor_mA = (v_pin32 / 0.22) * 1000.0;

    motor_mA = corriente_motor_mA;

}

void verificar_seguridad_motor() {
    medir_corriente_motor();

    static unsigned long tiempoFallaMotor = 0;
    static unsigned long tiempoFallaModulo = 0;

    // ====================================================
    // CASO A: DETECCIÓN DE MOTOR DESCONECTADO
    // ====================================================
    if (sistemaActivo && voltaje_12V >= 10.5 && motor_mA < 15.0) {
        if (tiempoFallaMotor == 0) tiempoFallaMotor = millis();

        if (millis() - tiempoFallaMotor >= 1500) { // 1.5 segundos de confirmación
            controlarCinta(PARAR);
            sistemaActivo = false;

            registrar_error(ERROR_MOTOR_DESCONECTADO,"!!! ERROR DE HARDWARE: EL MOTOR NO SE DETECTA !!! \n Amperaje: " + String(motor_mA) + "\n Voltaje de 12V: " + String(voltaje_12V));
            alarma_critica(255, 0, 255);
            tiempoFallaMotor = 0;
        }
    } else {
        tiempoFallaMotor = 0;
    }

    // ====================================================
    // CASO B: DETECCIÓN DE MÓDULO L298N O ENERGÍA DESCONECTADA
    // ====================================================

    if (sistemaActivo && voltaje_12V < 3.0) {
        if (tiempoFallaModulo == 0) tiempoFallaModulo = millis();

        if (millis() - tiempoFallaModulo >= 1500) {
            controlarCinta(PARAR);
            sistemaActivo = false;

            registrar_error(ERROR_L298N_SIN_ENERGIA,"!!! ERROR CRÍTICO: MÓDULO L298N O FUENTE PRINCIPAL DESCONECTADA !!!" + String(voltaje_12V));
            alarma_critica(255, 0, 255);
            tiempoFallaModulo = 0;
        }
    } else {
        tiempoFallaModulo = 0;
    }
}

void verificar_seguridad_pesaje() {
    static int lecturas_fallidas_consecutivas = 0; // Contador de errores

    // ====================================================
    // COMPROBACIÓN 1: ¿EL MÓDULO HX711 ESTÁ VIVO?
    // ====================================================
    // Incrementamos el timeout a 150ms para absorber retrasos del chip
    if (!bascula.wait_ready_timeout(150)) {
        lecturas_fallidas_consecutivas++; // Se detectó una anomalía, sumamos al contador

        // Si solo falló 1 o 2 veces seguidas, ignoramos el error momentáneamente (ruido o desfase)
        // Pero si falla 3 veces seguidas (aprox. 1 segundo continuo de error), el fallo es REAL.
        if (lecturas_fallidas_consecutivas >= 3) {
            if (hx711_conectado) {
                hx711_conectado = false;
                registrar_error(ERROR_HX711_DESCONECTADO,"!!! ERROR CRÍTICO: MÓDULO HX711 TOTALMENTE DESCONECTADO O QUEMADO !!!");

                controlarCinta(PARAR);
                sistemaActivo = false;
                alarma_critica(0,255,0);
            }
        }
        return; // Salimos para no leer basura
    }

    // Si el chip responde bien, limpiamos el contador de fallas de inmediato
    lecturas_fallidas_consecutivas = 0;
    hx711_conectado = true;

    // ====================================================
    // COMPROBACIÓN 2: ¿LA GALGA (CELDA) ESTÁ CONECTADA AL MÓDULO?
    // ====================================================
    static long ultima_lectura = 0;
    long lectura_actual = bascula.read();

    // Calculamos el salto absoluto entre esta lectura y la anterior
    long diferencia = labs(lectura_actual - ultima_lectura);
    ultima_lectura = lectura_actual;

    if (lectura_actual > 8000000 || lectura_actual < -8000000 || diferencia > 500000) {
        if (galga_saludable) {
            galga_saludable = false;
            registrar_error(ERROR_GALGA_CABLE_ROTO,"!!! ALERTA DE SENSOR: CABLE DE LA GALGA ROTO O CABLE DESCONECTADO !!!");

            // Puedes decidir si esto detiene tu cinta o solo manda una advertencia
            alarma_critica(0,255,0);
        }
    } else {
        galga_saludable = true;
    }
}

void salud_laser(int lacer) {
    if (!laser_saludable) return;

    uint16_t distancia;

    if (lacer != 0) {
        distancia = lacer;
    }else {

        distancia = laser.readRangeContinuousMillimeters();
    }


    // COMPROBACIÓN DUAL INDUSTRIAL:
    // retraso -> timeoutOccurred()
    // desborde -> 65535
    if (laser.timeoutOccurred() || distancia == 65535) {
        laser_saludable = false;

        registrar_error(ERROR_LASER_TIMEOUT,"\n!!! ERROR DE HARDWARE: SENSOR LÁSER VL53L0X PERDIÓ CONEXIÓN (TIMEOUT) !!!");

        controlarCinta(PARAR);
        sistemaActivo = false;
        alarma_critica(0,255,255);
        color_tira(tira.Color(0, 255, 255)); // Tira NeoPixel en color Cian fijo (Fallo Óptico)
        return;
    }
    laser_saludable = true;
}


void sonar_alarma(int pitos, int duracion) {
    for(int i = 0; i < pitos; i++) {
        digitalWrite(PIN_BUZZER, HIGH);
        delay(duracion);
        digitalWrite(PIN_BUZZER, LOW);
        delay(duracion);
    }
}

void color_tira(uint32_t color) {
    tira.setBrightness(50); // Brillo al 20% (Ahorro de energía)
    for(int i = 0; i < tira.numPixels(); i++) {
        tira.setPixelColor(i, color);
    }
    tira.show();
}

void color_led(int led, uint32_t color) {
    tira.setBrightness(50); // Brillo al 20% (Ahorro de energía)
    tira.setPixelColor(led, color);
    tira.show();
}

void color_punto_led(int led, uint32_t color) {
    tira.setBrightness(50); // Brillo al 20% (Ahorro de energía)
    tira.setPixelColor(led, color);
    tira.show();
    delay(20);
    tira.setPixelColor(led,tira.Color(0,0,255));
    tira.show();
}

void efecto_respirar(int r, int g, int b) {
    // Sube el brillo
    for (int i = 5; i < 50; i++) {
        tira.setBrightness(i);
        for(int i = 0; i < tira.numPixels(); i++) {
            tira.setPixelColor(i,tira.Color(r,g,b));
        }
        tira.show();
        delay(10);
    }
    // Baja el brillo
    for (int i = 50; i > 5; i--) {
        tira.setBrightness(i);
        for(int i = 0; i < tira.numPixels(); i++) {
            tira.setPixelColor(i,tira.Color(r,g,b));
        }
        tira.show();
        delay(10);
    }
}

void alarma_critica(int r, int g, int b) {
    for (int i = 0; i < 5; i++) {
        color_tira(tira.Color(255, 0, 0)); // Rojo brillante
        digitalWrite(PIN_BUZZER, HIGH);   // Sonido
        delay(100);

        color_tira(tira.Color(r, g, b));   // Apagado
        digitalWrite(PIN_BUZZER, LOW);    // Silencio
        delay(100);
    }
}

void Luz_carga(int i) {
    tira.setBrightness(50);

    int ledActual = i % 8;

    if (i < 8) {
        tira.setPixelColor(ledActual, tira.Color(255, 0, 255));
    } else if (i < 16) {
        tira.setPixelColor(ledActual, tira.Color(0, 255, 0));
    } else {
        tira.setPixelColor(ledActual, tira.Color(0, 0, 255));
    }

    tira.show();
}

void avisos_led() {
    tira.setBrightness(50);

    if (sistemaActivo){
        tira.setPixelColor(0, tira.Color(0, 0, 255));
    }else {
        tira.setPixelColor(0, tira.Color(255, 0, 0));
    }

    if (WiFi.status() == WL_CONNECTED) {
        tira.setPixelColor(1, tira.Color(0, 0, 255));
        internet = true;
    }else {
        tira.setPixelColor(1, tira.Color(255, 0, 0));
        internet = false;
        if (online) {
            alarma_critica(255,0,0);
            online = false;
        }
    }

    if (online) {
        tira.setPixelColor(2, tira.Color(0, 0, 255));
    }else {
        tira.setPixelColor(2, tira.Color(255, 0, 0));
    }

    if (serverActivo && internet){
        tira.setPixelColor(6, tira.Color(0, 0, 255));
    }else {
        if (online) {
            alarma_critica(255,0,255);
        }
        tira.setPixelColor(6, tira.Color(255, 0, 0));
    }

    tira.show();
}