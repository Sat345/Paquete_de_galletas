//
// Created by sait1 on 02/05/2026.
//

#ifndef GLOBALES_H
#define GLOBALES_H

#include <Arduino.h>
#include <WebServer.h>

// Variables que todos van a leer y escribir

extern bool sistemaActivo;
extern bool serverActivo;
extern bool objeto;
extern bool midiendo;
extern bool online;
extern bool internet;
extern bool hx711_conectado;
extern bool galga_saludable;
extern bool laser_saludable;
extern WebServer server;
extern String categoria_actual;
extern int ultimo_error_codigo;
extern int contador_chicas;
extern int contador_medianas;
extern int contador_grandes;
extern int no_validas;
extern float grosor_maximo;
extern float peso;
extern float pesos;
extern float tamanio;                   // t=2s v=10cm*s  tamaño =  10/2  = 5cm
extern float voltaje_5V;
extern float corriente_5V;
extern float resistencia_5V;
extern float voltaje_12V;
extern float corriente_12V;
extern float resistencia_12V;
extern float motor_mA;
extern unsigned long tiempo_entrada;
extern
unsigned long tiempo_en_pasar;
extern float distancia_lector;
extern float altitud_cinta;
extern const float umbral_largo;
extern const float umbral_peso;


enum EstadoProceso {
    ESPERA,
    SUBIENDO,
    BAJANDO,
    MIDIENDO,
    CLASIFICANDO
};
extern EstadoProceso estado_proceso;
enum Escoger_brazo {PRIMER_BRAZO, SEGUNDO_BRAZO, TERCER_BRAZO};
extern bool brazo_en_movimiento;
extern unsigned long tiempo_inicio_brazo;
extern Escoger_brazo brazo_activo;

extern unsigned long tiempo_corto;
extern unsigned long tiempo_medio;
extern unsigned long tiempo_largo;

extern const long intervalo_sensores;
extern const long intervalo_corto;
extern const long intervalo_medio;
extern const long intervalo_largo;

#endif