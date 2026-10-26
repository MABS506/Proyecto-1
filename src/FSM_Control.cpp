// ====================================================
// FSM_Control.cpp - Acciones y Transiciones de la FSM 
// ====================================================

#include "FSM_Control.h"
#include "HAL_Motor.h"

constexpr uint32_t VELOCIDAD_MARCHA = 200;

// Funciones Estados (Acciones del Motor)
void onNoOp() {}

void onStart() {
    motorPWM(VELOCIDAD_MARCHA);
}

void onFault() {
    motorStop();
}

void onReset() {
    motorStop();
}

// Matriz de Acceso Estado - Evento
constexpr Transition TransitionFSM [static_cast <size_t> (State::COUNT)][static_cast <size_t> (Event::COUNT)] = {
    //            |           NONE           |           START           |           STOP           |           RESET
    /* IDLE    */ { {State::IDLE,    onNoOp}, {State::RUNNING, onStart}, {State::FAULT,   onFault}, {State::IDLE,    onNoOp} },
    /* RUNNING */ { {State::RUNNING, onNoOp}, {State::RUNNING, onNoOp},  {State::FAULT,   onFault}, {State::RUNNING, onNoOp} },
    /* FAULT   */ { {State::FAULT,   onNoOp}, {State::FAULT,   onNoOp},  {State::FAULT,   onNoOp},  {State::IDLE,    onReset} }
};
