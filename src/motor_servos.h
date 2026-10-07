//
// Created by sait1 on 02/05/2026.
//

#ifndef MOTOR_SERVOS_H
#define MOTOR_SERVOS_H

#include <Arduino.h>
#include "GLOBALES.h"
#include <ESP32Servo.h>

// Definimos los pines aquí para que estén ordenados
const int PIN_ENA = 14;
const int MOTOR_IN1 = 25;
const int MOTOR_IN2 = 26;

const int PIN_ENB = 23;
const int MOTOR_IN3 = 19;
const int MOTOR_IN4 = 5;

const int PIN_BRAZO_CHICO = 18;
const int PIN_BRAZO_MEDIANO = 33;
const int PIN_BRAZO_GRANDE = 27;// Servo de desvío

extern Servo brazo_chico;
extern Servo brazo_mediano;
extern Servo brazo_grande;

const int velocidad_A = 255;
const int velocidad_B = 50;

enum Estado_motor {PARAR, ADELANTE, ATRAS};

void controlarCinta(Estado_motor modo);
void motor_B(Estado_motor modo);
void controlarBrazo(Escoger_brazo brazo_escogido);
void actualizarBrazo();

#endif
