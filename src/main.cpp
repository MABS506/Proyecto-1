#include <Arduino.h>
#include <xtensa/core-macros.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "FSM_Control.h"
#include "HAL_Motor.h"

// ----------------
//  Mapeo de pines
// ----------------
constexpr uint8_t PIN_PARADA_EMERGENCIA = 33;   // microswitch, pull-up interna, activo en bajo

// ------------------------------------------------------
//  Antirrebote: 4 000 000 ciclos = 25 ms SOLO a 160 MHz
// ------------------------------------------------------
constexpr uint32_t DEBOUNCE_CYCLES = 4000000UL;

// ----------------------------
//  Sincronizacion ISR ↔ tarea
// ----------------------------
static SemaphoreHandle_t   semParada = nullptr;                 
static portMUX_TYPE        isrMux = portMUX_INITIALIZER_UNLOCKED;
static volatile uint32_t   isrPressCount = 0;                   
static uint32_t            isrLastCycle = 0;                    

static uint32_t leerPulsaciones() {
    portENTER_CRITICAL(&isrMux);
    const uint32_t c = isrPressCount;
    portEXIT_CRITICAL(&isrMux);
    return c;
}

// --------------------------------
//  ISR de la Parada de Emergencia
// --------------------------------
static void IRAM_ATTR isrParada() {
    const uint32_t ahora = XTHAL_GET_CCOUNT();

    if ((uint32_t)(ahora - isrLastCycle) < DEBOUNCE_CYCLES) {
        return;
    }
    isrLastCycle = ahora;

    portENTER_CRITICAL_ISR(&isrMux);
    isrPressCount++;
    portEXIT_CRITICAL_ISR(&isrMux);

    BaseType_t desperto = pdFALSE;
    xSemaphoreGiveFromISR(semParada, &desperto);
    if (desperto == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

// ---------------------------------------------------
//  FSM - Transiciones del Motor por Cambio de Estado 
// ---------------------------------------------------
static const char *const NOMBRE_ESTADO[] = { "IDLE", "RUNNING", "FAULT" };
static const char *const NOMBRE_EVENTO[] = { "NONE", "START", "STOP", "RESET" };

static void despachar(State &estado, Event evento) {
    const size_t Estado = static_cast<size_t>(estado);
    const size_t Evento = static_cast<size_t>(evento);

    const Transition &t = TransitionFSM[Estado][Evento];

    t.action();

    if (t.nextState != estado) {
        Serial.printf("[FSM] %s --(%s)--> %s\n",
                      NOMBRE_ESTADO[Estado], NOMBRE_EVENTO[Evento],
                      NOMBRE_ESTADO[static_cast<size_t>(t.nextState)]);
    }
    estado = t.nextState;
}

static Event eventoDesdeSerial(char c) {
    switch (c) {
        case 's': case 'S': return Event::START;
        case 'x': case 'X': return Event::STOP;
        case 'r': case 'R': return Event::RESET;
        default:            return Event::NONE;
    }
}

// -----------------
//  Tarea de la FSM
// -----------------
static void taskFSM(void *) {
    State estado = State::IDLE;
    Serial.println("[FSM] listo en IDLE. Comandos: s = START, x = STOP, r = RESET");

    for (;;) {
        if (xSemaphoreTake(semParada, pdMS_TO_TICKS(10)) == pdTRUE) {
            despachar(estado, Event::STOP);
            Serial.printf("[ISR] parada de emergencia (pulsaciones: %lu)\n",
                          static_cast<unsigned long>(leerPulsaciones()));
        }

        while (Serial.available() > 0) {
            const Event ev = eventoDesdeSerial(static_cast<char>(Serial.read()));
            if (ev != Event::NONE) {
                despachar(estado, ev);
            }
        }
    }
}

// -------------------
//  Super Bucle/Ciclo 
// -------------------
void setup() {
    Serial.begin(115200);
    motorInit();

    semParada = xSemaphoreCreateBinary();
    const bool tareaOk = (semParada != nullptr) &&
        (xTaskCreatePinnedToCore(taskFSM, "FSM", 4096, nullptr, 3, nullptr, 1) == pdPASS);

    if (!tareaOk) {
        Serial.println("[ERROR] no se pudo crear el semaforo o la tarea FSM");
        motorStop();
        return;
    }

    pinMode(PIN_PARADA_EMERGENCIA, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_PARADA_EMERGENCIA), isrParada, FALLING);
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}
