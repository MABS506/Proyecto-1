// =================================================================
// HAL_Control.cpp - Implementación de la Capa de Control del Motor 
// =================================================================

#include <Arduino.h>
#include <hal_motor.h>

constexpr uint8_t  MOTOR_A_PIN = 12; 
constexpr uint8_t  MOTOR_B_PIN = 14; 

void motorInit(){
    pinMode(MOTOR_A_PIN, OUTPUT);
    pinMode(MOTOR_B_PIN, OUTPUT);
    motorStop();

};
void motorPWM(uint32_t cicloTrabajo){
    if(cicloTrabajo > 255){
        cicloTrabajo = 255;
    }
    analogWrite(MOTOR_A_PIN, cicloTrabajo);
    digitalWrite(MOTOR_B_PIN, LOW);
    
};
void motorStop(){
    analogWrite(MOTOR_A_PIN, 0);
    digitalWrite(MOTOR_B_PIN, LOW);
};