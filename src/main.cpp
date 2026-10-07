#include <Arduino.h>
#include <Wire.h>
#include "HX711.h"
#include <VL53L0X.h>
#include <ESP32Servo.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include "motor_servos.h"
#include "comunicaciones.h"
#include "GLOBALES.h"
#include "logica.h"
#include "sensores.h"
#include "diagnostico.h"

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(30000);

    tira.begin();
    color_tira(tira.Color(255, 0, 0)); // Rojo = arrancando
    pinMode(PIN_BUZZER, OUTPUT);

    // Configuración selectiva de pines analógicos:

    analogSetPinAttenuation(32, ADC_0db);
    analogSetPinAttenuation(35, ADC_11db);

    // Iniciar Sensores
    pinMode(SDA_PIN, INPUT_PULLUP);
    pinMode(SCL_PIN, INPUT_PULLUP);

    Wire.begin();
    Wire.setTimeOut(50);

    if (!laser.init()) {
        registrar_error(ERROR_LASER_NO_ENCONTRADO,"!!! ERROR CRÍTICO: NO SE ENCONTRÓ EL SENSOR VL53L0X AL ARRANCAR !!!");
        laser_saludable = false;
    } else {
        laser_saludable = true;
        laser.setTimeout(200); // Define un tiempo límite de espera de 200ms
        laser.startContinuous(); // Inicia mediciones de distancia continuas
    }

    // Inicializar Motor Cinta
    pinMode(MOTOR_IN1, OUTPUT);
    pinMode(MOTOR_IN2, OUTPUT);
    pinMode(PIN_ENA, OUTPUT);

    pinMode(MOTOR_IN3, OUTPUT);
    pinMode(MOTOR_IN4, OUTPUT);
    pinMode(PIN_ENB, OUTPUT);

    // Iniciar Servo
    brazo_chico.attach(PIN_BRAZO_CHICO);
    brazo_mediano.attach(PIN_BRAZO_MEDIANO);
    brazo_grande.attach(PIN_BRAZO_GRANDE);
    brazo_chico.write(0);
    brazo_mediano.write(0);
    brazo_grande.write(0);

    bascula.begin(DT_PIN, SCK_PIN);
    bascula.set_scale(410.0);
    bascula.tare();

    // Calibracion automatica de altura
    delay(500);
    altitud_cinta = laser.readRangeContinuousMillimeters();
    Serial.print("La altitud de la sinta es: ");
    Serial.println(altitud_cinta);

    pinMode(PIN_BUZZER, OUTPUT);

    WiFi.setTxPower(WIFI_POWER_8_5dBm);
    conectar_wifi();
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("¡Estas conectado a internet!");
        online = true;
    } else {
        Serial.println("Se perdio la coneccion, si quieres volver a intentarlo presiona 'W'");
        Serial.println("Si quieres cambiar la cambiar los datos del wifi presiona 'I'");
        Serial.println("entrando a modo offline");
        online = false;
    }

    color_tira(tira.Color(0, 0, 255)); // Azul al iniciar
    bascula.tare();
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        server.handleClient();
    }

    if (Serial.available() > 0) {
        char comando = Serial.read(); //leer
        switch (comando) {
            case 'V':
                Serial.println("V: Visualizar las opciones");
                Serial.println("A: Activar sistema");
                Serial.println("P: Parar sistema");
                Serial.println("4: Avanzar motor");
                Serial.println("5: Parar motor");
                Serial.println("6: Retroceder motor");
                Serial.println("L: Medir distancia lacer");
                Serial.println("T: Tarar bascula");
                Serial.println("B: Obtener peso");
                Serial.println("D: Mostrar datos");
                Serial.println("F: Mostrar fecha");
                Serial.println("W: Conectar wifi");
                Serial.println("I: Cambiar nombre y contraseña de el wifi");
                Serial.println("C: Alarma critica");
                Serial.println("O: Activar modo offline");
                Serial.println("1: Probar brazo 1");
                Serial.println("2: Probar brazo 2");
                Serial.println("3: Probar brazo 3");
                break;
            case 'A':
                sistemaActivo = true;
                controlarCinta(ADELANTE);
                Serial.println("Sistema Activo");
                break;
            case 'P':
                sistemaActivo = false;
                controlarCinta(PARAR);
                Serial.println("Sistema Detenido");
                break;
            case '4':
                controlarCinta(ADELANTE);
                break;
            case '5':
                controlarCinta(PARAR);
                break;
            case '6':
                controlarCinta(ATRAS);
                break;
            case '7':
                motor_B(ADELANTE);
                break;
            case '8':
                motor_B(PARAR);
                break;
            case '9':
                motor_B(ATRAS);
                break;
            case 'L':
                Serial.print("Distancia: ");
                Serial.print(usar_laser());
                Serial.println(" mm");
                break;
            case 'T':
                bascula.tare();
                Serial.println("Bascula puesta en 0");
                break;
            case 'B':
                Serial.print("Peso bascula obtenido: ");
                Serial.print(obtenerPeso());
                Serial.println(" g");
                break;
            case 'D':
                obtenerValores();
                Serial.println();
                Serial.println("Datos mostrados");
                break;
            case 'F':
                Serial.print("La fecha es: ");  Serial.println(obtenerFechaHora());
                break;
            case 'W':
                conectar_wifi();
                if (WiFi.status() == WL_CONNECTED) {
                    Serial.println("¡Estas conectado a internet!");
                } else {
                    Serial.println("Se perdio la coneccion");
                }
                break;
            case 'I':
                configurar_wifi_manual();
                break;
            case 'C':
                alarma_critica(0,0,0);
                break;
            case 'O':
                online = false;
                Serial.println("Modo offline activado");
                break;
            case 'M':
                medir_corriente_motor();
                Serial.print("Amperaje de el motor: "); Serial.print(motor_mA); Serial.println(" mA");
                break;
            case '1':
                controlarBrazo(PRIMER_BRAZO);
                Serial.println("Brazo 1 probado");
                break;
            case '2':
                controlarBrazo(SEGUNDO_BRAZO);
                Serial.println("Brazo 2 probado");
                break;
            case '3':
                controlarBrazo(TERCER_BRAZO);
                Serial.println("Brazo 3 probado");
                break;
            default: ;
        }
    }

    if (sistemaActivo) {
        procesar();
    }

    actualizarBrazo();

    if (millis() - tiempo_corto >= intervalo_corto) {
        avisos_led();
        tiempo_corto = millis();
    }

    /*
    if (millis() - tiempo_medio >= intervalo_medio) {
        verificarSaludEnergia();
        tiempo_medio = millis();
    }

    if (millis() - tiempo_largo >= intervalo_largo) {
        //verificarServidor();  Hace el ping
        verificar_seguridad_pesaje();
        verificar_seguridad_motor();
        salud_laser(0);
        tiempo_largo = millis();
    }
    */
}


