#include "gate.h"
#include <ESP32Servo.h>

static Servo _servo_entry;   // Standard positional servo
static Servo _servo_exit;    // 360° continuous rotation servo

static bool _entry_open = false;
static bool _exit_open  = false;

void gate_init() {
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);

    // ── Entry gate: standard positional servo ──────────────────────────────────
    _servo_entry.setPeriodHertz(50);
    _servo_entry.attach(PIN_SERVO_ENTRY, 500, 2400);
    _servo_entry.write(SERVO_ENTRY_CLOSED);   // Start closed

    // ── Exit gate: 360° continuous rotation servo ──────────────────────────────
    // Pulse range 1000–2000µs is typical for 360 servos
    // 1500µs (value 90) = stop dead centre
    _servo_exit.setPeriodHertz(50);
    _servo_exit.attach(PIN_SERVO_EXIT, 1000, 2000);
    _servo_exit.write(SERVO_EXIT_STOP);       // Ensure stopped at init

    Serial.println("[GATE] Entry: positional servo — Exit: 360 continuous servo");
    Serial.println("[GATE] Both gates initialized CLOSED");
}

// ── Entry gate (positional) ────────────────────────────────────────────────────

static void _entry_open_gate() {
    if (_entry_open) return;
    Serial.println("[GATE] Entry gate OPENING");
    _servo_entry.write(SERVO_ENTRY_OPEN);
    vTaskDelay(pdMS_TO_TICKS(600));   // Wait for positional servo to reach angle
    _entry_open = true;
}

static void _entry_close_gate() {
    if (!_entry_open) return;
    Serial.println("[GATE] Entry gate CLOSING");
    _servo_entry.write(SERVO_ENTRY_CLOSED);
    vTaskDelay(pdMS_TO_TICKS(600));
    _entry_open = false;
}

// ── Exit gate (360° continuous rotation) ──────────────────────────────────────
// The 360 servo has no position feedback — we run it for a fixed duration
// then stop. SERVO_EXIT_OPEN_MS and SERVO_EXIT_CLOSE_MS in config.h control
// how long it runs. Tune these to match your physical gate arm travel.

static void _exit_open_gate() {
    if (_exit_open) return;
    Serial.printf("[GATE] Exit gate OPENING — running at speed %d for %dms\n",
                  SERVO_EXIT_OPEN_SPEED, SERVO_EXIT_OPEN_MS);

    _servo_exit.write(SERVO_EXIT_OPEN_SPEED);          // Start spinning (open direction)
    vTaskDelay(pdMS_TO_TICKS(SERVO_EXIT_OPEN_MS));     // Run for timed duration
    _servo_exit.write(SERVO_EXIT_STOP);                // Stop
    vTaskDelay(pdMS_TO_TICKS(100));                    // Brief settle

    _exit_open = true;
    Serial.println("[GATE] Exit gate OPEN");
}

static void _exit_close_gate() {
    if (!_exit_open) return;
    Serial.printf("[GATE] Exit gate CLOSING — running at speed %d for %dms\n",
                  SERVO_EXIT_CLOSE_SPEED, SERVO_EXIT_CLOSE_MS);

    _servo_exit.write(SERVO_EXIT_CLOSE_SPEED);         // Start spinning (close direction)
    vTaskDelay(pdMS_TO_TICKS(SERVO_EXIT_CLOSE_MS));    // Run for timed duration
    _servo_exit.write(SERVO_EXIT_STOP);                // Stop
    vTaskDelay(pdMS_TO_TICKS(100));                    // Brief settle

    _exit_open = false;
    Serial.println("[GATE] Exit gate CLOSED");
}

// ── Public API — same interface as before, main.cpp unchanged ─────────────────

void gate_open(GateId gate) {
    if (gate == GATE_ENTRY) _entry_open_gate();
    else                    _exit_open_gate();
}

void gate_close(GateId gate) {
    if (gate == GATE_ENTRY) _entry_close_gate();
    else                    _exit_close_gate();
}

bool gate_is_open(GateId gate) {
    return gate == GATE_ENTRY ? _entry_open : _exit_open;
}
