#include "motor.h"

static bool _gate_open = false;

void motor_init() {
    pinMode(PIN_MOTOR_IN1, OUTPUT);
    pinMode(PIN_MOTOR_IN2, OUTPUT);
    pinMode(PIN_MOTOR_ENA, OUTPUT);
    motor_stop();
    Serial.println("[MOTOR] Ready");
}

void motor_open_gate() {
    if (_gate_open) return;
    Serial.println("[MOTOR] Opening gate");
    digitalWrite(PIN_MOTOR_IN1, HIGH);
    digitalWrite(PIN_MOTOR_IN2, LOW);
    analogWrite(PIN_MOTOR_ENA, 200);   // ~78% speed
    vTaskDelay(pdMS_TO_TICKS(1500));   // Run for 1.5s to lift arm
    motor_stop();
    _gate_open = true;
}

void motor_close_gate() {
    if (!_gate_open) return;
    Serial.println("[MOTOR] Closing gate");
    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, HIGH);
    analogWrite(PIN_MOTOR_ENA, 200);
    vTaskDelay(pdMS_TO_TICKS(1500));
    motor_stop();
    _gate_open = false;
}

void motor_stop() {
    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, LOW);
    analogWrite(PIN_MOTOR_ENA, 0);
}

bool motor_is_open() { return _gate_open; }
