// =========================================================
// HAL_Motor.h - Capa de Abstracción del Puente H (L91100S)
// =========================================================

#include <stdint.h>
#include <Arduino.h>

void motorInit();
void motorPWM(uint32_t cicloTrabajo);
void motorStop();