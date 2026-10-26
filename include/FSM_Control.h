// =============================================
// FSM_control.h - Maquina de Estados del Motor 
// =============================================

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

enum class State: uint8_t { IDLE = 0, RUNNING, FAULT, COUNT};
enum class Event: uint8_t { NONE = 0, START, STOP, RESET, COUNT};

struct Transition {
    State nextState;
    void (*action)();
};

extern const Transition TransitionFSM[static_cast<size_t>(State::COUNT)][static_cast<size_t>(Event::COUNT)];

extern void onStart();
extern void onFault();
extern void onReset();
extern void onNoOp();