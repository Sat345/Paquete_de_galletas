//
// Created by sait1 on 02/05/2026.
//

#include "motor_servos.h"
#include "GLOBALES.h"

Servo brazo_chico;
Servo brazo_mediano;
Servo brazo_grande;

void controlarCinta(Estado_motor modo) {

    static Estado_motor ultimoModo = PARAR;

    if (modo != ultimoModo && modo != PARAR) {
        analogWrite(PIN_ENA, 0);
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, LOW);
        delay(1700); // Pequeña pausa para proteger los engranajes y el driver
    }
    ultimoModo = modo;

    if (modo == PARAR) {
        analogWrite(PIN_ENA, 0);
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, LOW);

    }else if (modo == ATRAS) {
        analogWrite(PIN_ENA, velocidad_A);
        digitalWrite(MOTOR_IN1, HIGH);
        digitalWrite(MOTOR_IN2, LOW);

    }else {
        analogWrite(PIN_ENA, velocidad_A);
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, HIGH);
    }
}

void motor_B(Estado_motor modo) {

    static Estado_motor ultimoModo = PARAR;

    if (modo != ultimoModo && modo != PARAR) {
        analogWrite(PIN_ENB, 0);
        digitalWrite(MOTOR_IN3, LOW);
        digitalWrite(MOTOR_IN4, LOW);
        delay(1700); // Pequeña pausa para proteger los engranajes y el driver
    }
    ultimoModo = modo;

    if (modo == PARAR) {
        analogWrite(PIN_ENB, 0);
        digitalWrite(MOTOR_IN3, LOW);
        digitalWrite(MOTOR_IN4, LOW);

    }else if (modo == ATRAS) {
        analogWrite(PIN_ENB, velocidad_B);
        digitalWrite(MOTOR_IN3, HIGH);
        digitalWrite(MOTOR_IN4, LOW);

    }else {
        analogWrite(PIN_ENB, velocidad_B);
        digitalWrite(MOTOR_IN3, LOW);
        digitalWrite(MOTOR_IN4, HIGH);
    }
}



void controlarBrazo(Escoger_brazo brazo_escogido) {
    int angulo = 45;
    if (brazo_escogido == PRIMER_BRAZO)       brazo_chico.write(angulo);
    else if (brazo_escogido == SEGUNDO_BRAZO) brazo_mediano.write(angulo);
    else                                       brazo_grande.write(angulo);

    brazo_en_movimiento = true;
    tiempo_inicio_brazo = millis();
    brazo_activo = brazo_escogido;
}

void actualizarBrazo() {
    if (brazo_en_movimiento && millis() - tiempo_inicio_brazo >= 1000) {
        if (brazo_activo == PRIMER_BRAZO)       brazo_chico.write(0);
        else if (brazo_activo == SEGUNDO_BRAZO) brazo_mediano.write(0);
        else                                     brazo_grande.write(0);

        brazo_en_movimiento = false;
    }
}