#include "gate.h"
#include <ESP32Servo.h>

// Two servo objects — one per gate
static Servo _servo_entry;
static Servo _servo_exit;

static bool _entry_open = false;
static bool _exit_open  = false;

void gate_init() {
    // Allocate LEDC PWM channels for each servo
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);

    _servo_entry.setPeriodHertz(50);   // Standard servo 50Hz
    _servo_exit.setPeriodHertz(50);

    _servo_entry.attach(PIN_SERVO_ENTRY, 500, 2400);  // Min/max pulse width µs
    _servo_exit.attach(PIN_SERVO_EXIT,   500, 2400);

    // Ensure both gates start closed
    _servo_entry.write(180);
    _servo_exit.write(SERVO_GATE_CLOSED);

    Serial.println("[GATE] Entry and exit servos ready — both closed");
}

void gate_open(GateId gate) {
    if (gate == GATE_ENTRY) {
        if (_entry_open) return;
        Serial.println("[GATE] Entry gate OPENING");
        _servo_entry.write(SERVO_GATE_OPEN);
        _entry_open = true;
    } else {
        if (_exit_open) return;
        Serial.println("[GATE] Exit gate OPENING");
        _servo_exit.write(SERVO_GATE_OPEN);
        _exit_open = true;
    }
    vTaskDelay(pdMS_TO_TICKS(500));   // Give servo time to reach position
}

void gate_close(GateId gate) {
    if (gate == GATE_ENTRY) {
        if (!_entry_open) return;
        Serial.println("[GATE] Entry gate CLOSING");
        _servo_entry.write(180);
        _entry_open = false;
    } else {
        if (!_exit_open) return;
        Serial.println("[GATE] Exit gate CLOSING");
        _servo_exit.write(SERVO_GATE_CLOSED);
        _exit_open = false;
    }
    vTaskDelay(pdMS_TO_TICKS(500));
}

bool gate_is_open(GateId gate) {
    return gate == GATE_ENTRY ? _entry_open : _exit_open;
}
