//
// Created by sait1 on 02/05/2026.
//

#ifndef UNTITLED1_COMUNICACIONES_H
#define UNTITLED1_COMUNICACIONES_H

#include <Arduino.h>
#include <Preferences.h>

extern Preferences preferences;

extern const char* ssid;
extern const char* password;
extern const char* ntpServer;

void configurarReloj();
String obtenerFechaHora();
void conectar_wifi();
void guardar_wiFi(String s, String p);
void configurar_wifi_manual();
String generarJSON();
void obtenerValores();
void enviar_a_base_de_datos();
void configurarRutas();
void enviar_error_a_base_de_datos(int codigo, String mensaje);

#endif //UNTITLED1_COMUNICACIONES_H