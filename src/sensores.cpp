//
// Created by sait1 on 02/05/2026.
//

#include "sensores.h"

#include "diagnostico.h"

HX711 bascula;
VL53L0X laser;
float factor_calibracion = 0.00243976965882261091024609050823;

float obtenerPeso() {
    float p = 9999999;
    unsigned long tiempoLimite = millis();

    // Esperar 200ms
    while (!bascula.is_ready() && (millis() - tiempoLimite < 200)) {
        // No hace nada, solo espera
    }

    if (bascula.is_ready()) {
        p = bascula.get_units(5); // Promedio de 5 lecturas
    }
    return p;
}

int usar_laser() {
    int distancia = laser.readRangeContinuousMillimeters();

    if (laser.timeoutOccurred()) {
        Serial.print(" TIMEOUT");
    }

    salud_laser(distancia);
    return distancia;
}

