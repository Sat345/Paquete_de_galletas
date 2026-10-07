//
// Created by sait1 on 02/05/2026.
//

#ifndef UNTITLED1_DIAGNOSTICO_H
#define UNTITLED1_DIAGNOSTICO_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>



const int PIN_VOLTAJE = 34;     // Pin analógico
const int PIN_BUZZER = 13;   // Pin para el zumbador
const int PIN_LEDS = 4;      // Pin para la tira de LEDs
const int NUM_LEDS = 8;      // Cantidad de Lets

extern Adafruit_NeoPixel tira;

void medir_5V();
void medir_12V();
void medir_corriente_motor();
void verificar_seguridad_motor();
void verificarSaludEnergia();
void verificar_seguridad_pesaje();
void salud_laser(int lacer);
void sonar_alarma(int pitos, int duracion);
void efecto_respirar(int r, int g, int b);
void alarma_critica(int r, int g, int b);
void color_led(int led, uint32_t color);
void color_punto_led(int led, uint32_t color);
void color_tira(uint32_t color);
void Luz_carga(int i);
void avisos_led();

// Catálogo de errores del sistema
enum CodigoError {
    SIN_ERROR = 0,
    ERROR_MOTOR_DESCONECTADO    = 101,
    ERROR_L298N_SIN_ENERGIA     = 102,
    ERROR_HX711_DESCONECTADO    = 201,
    ERROR_GALGA_CABLE_ROTO      = 202,
    ERROR_LASER_TIMEOUT         = 301,
    ERROR_LASER_NO_ENCONTRADO   = 302,
    ERROR_BAJO_VOLTAJE_5V       = 401,
    ERROR_ALTO_VOLTAJE_5V       = 402,
    ERROR_BAJA_CORRIENTE_5V     = 403,
    ERROR_ALTA_CORRIENTE_5V     = 404,
    ERROR_BAJO_VOLTAJE_12V      = 405,
    ERROR_ALTO_VOLTAJE_12V      = 406,
    ERROR_BAJA_RESISTENCIA_12V  = 407,
    ERROR_ALTA_RESISTENCIA_12V  = 408,
    ERROR_ALTA_CORRIENTE_12V    = 409,
    ERROR_BAJA_CORRIENTE_12V    = 410,
    ERROR_TIEMPO                = 501,
    ERROR_CONECCION_WIFI        = 601,
    ERROR_DATOS_WIFI_INCORRECTOS= 602,
    ERROR_ENVIAR_DATOS_SERVIDOR = 603,
    ERROR_SIN_CONECCION_SERVIDOR= 604,
    Error_mensaje_sin_valor     =701

};


void registrar_error(CodigoError codigo, String mensaje);

#endif //UNTITLED1_DIAGNOSTICO_H