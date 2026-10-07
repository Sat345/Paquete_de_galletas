//
// Created by sait1 on 02/05/2026.
//

#include "GLOBALES.h"

bool sistemaActivo = false;
bool serverActivo = true;
bool objeto = false;
bool midiendo = false;
bool online = false;
bool internet = false;

bool hx711_conectado = true;
bool galga_saludable = true;
bool laser_saludable = true;

WebServer server(80);
String categoria_actual = "";
int ultimo_error_codigo = 0;
int contador_chicas = 0;
int contador_medianas = 0;
int contador_grandes = 0;
int no_validas = 0;
float grosor_maximo = 0;
float peso = 0;
float pesos = 0;
float tamanio = 0;                      // t=2s v=10cm*s  tamaño =  10/2  = 5cm
float voltaje_5V = 0;
float corriente_5V = 0;
float resistencia_5V = 0;
float voltaje_12V = 0;
float corriente_12V = 0;
float resistencia_12V = 0;
float motor_mA = 0;
unsigned long tiempo_entrada = 0;
unsigned long tiempo_en_pasar;
float distancia_lector = 0;
float altitud_cinta = 0;        // Distancia del láser a la cinta en mm
const float umbral_largo = 3.0;
const float umbral_peso = 3.0;  //agustar

bool brazo_en_movimiento = false;
unsigned long tiempo_inicio_brazo = 0;
Escoger_brazo brazo_activo = PRIMER_BRAZO;  // ← valor inicial
EstadoProceso estado_proceso = ESPERA;

unsigned long tiempo_corto = 0;
unsigned long tiempo_medio = 0;
unsigned long tiempo_largo = 0;

const long intervalo_sensores = 50; //
const long intervalo_corto = 100;   // 0.1s
const long intervalo_medio = 1000;  // 1s
const long intervalo_largo = 5000;  // 5s

