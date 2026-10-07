//
// Created by sait1 on 02/05/2026.
//

#ifndef UNTITLED1_SENSORES_H
#define UNTITLED1_SENSORES_H

#include <Arduino.h>
#include "HX711.h"
#include <VL53L0X.h>

extern HX711 bascula;
extern VL53L0X laser;

//bascula
const int DT_PIN = 16;
const int SCK_PIN = 17;
//
const int SDA_PIN = 21;
const int SCL_PIN = 22;

extern float factor_calibracion; //primer_resultado_obtenido/peso_real=factor_de_calibracion_real

float obtenerPeso();
int usar_laser();

#endif //UNTITLED1_SENSORES_H