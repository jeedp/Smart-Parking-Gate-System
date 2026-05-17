#include "ultrasonic.h"
#include "config.h"

// ─── Sensor reading cache ──────────────────────────────────────────────────────
// ultrasonic_read_cm() is a blocking call (pulseIn up to 30ms).
// We cache the last reading so the main task can check it without
// waiting for a fresh pulse every single loop iteration.
static volatile float _entry_cm = -1.0f;
static volatile float _exit_cm  = -1.0f;

// ─── Internal single read ─────────────────────────────────────────────────────
static float _read_once(uint8_t trig, uint8_t echo) {
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    long duration = pulseIn(echo, HIGH, ULTRASONIC_TIMEOUT_US);
    if (duration == 0) return -1.0f;
    return (duration * 0.0343f) / 2.0f;
}

// ─── Background polling task (Core 0, low priority) ───────────────────────────
// Runs independently — reads both sensors every ULTRASONIC_POLL_MS and caches
// results so the gate control task never blocks on pulseIn.
static void task_ultrasonic_poll(void*) {
    for (;;) {
        _entry_cm = _read_once(PIN_ULTRASONIC_ENTRY_TRIG, PIN_ULTRASONIC_ENTRY_ECHO);
        vTaskDelay(pdMS_TO_TICKS(20));   // Small gap between sensors to avoid interference
        _exit_cm  = _read_once(PIN_ULTRASONIC_EXIT_TRIG,  PIN_ULTRASONIC_EXIT_ECHO);
        vTaskDelay(pdMS_TO_TICKS(ULTRASONIC_POLL_MS - 20));
    }
}

void ultrasonic_init() {
    pinMode(PIN_ULTRASONIC_ENTRY_TRIG, OUTPUT);
    pinMode(PIN_ULTRASONIC_ENTRY_ECHO, INPUT);
    digitalWrite(PIN_ULTRASONIC_ENTRY_TRIG, LOW);

    pinMode(PIN_ULTRASONIC_EXIT_TRIG, OUTPUT);
    pinMode(PIN_ULTRASONIC_EXIT_ECHO, INPUT);
    digitalWrite(PIN_ULTRASONIC_EXIT_TRIG, LOW);

    // Spawn background polling task on Core 0 at low priority (1)
    // so it never blocks the gate control task (priority 5)
    xTaskCreatePinnedToCore(
        task_ultrasonic_poll,
        "UltrasonicPoll",
        2048,
        nullptr,
        1,        // Low priority — yields to GateControlTask immediately
        nullptr,
        0         // Core 0 alongside GateControlTask
    );

    Serial.println("[ULTRASONIC] Sensors ready — background polling started");
}

// ─── Public API — all non-blocking, read from cache ───────────────────────────

float ultrasonic_read_cm(SensorId sensor) {
    return (sensor == SENSOR_ENTRY) ? _entry_cm : _exit_cm;
}

bool ultrasonic_car_present(SensorId sensor) {
    float dist = ultrasonic_read_cm(sensor);
    if (dist < 0) return false;
    return dist < ULTRASONIC_TRIGGER_CM;
}

bool ultrasonic_wait_car_clear(SensorId sensor, uint32_t timeout_ms) {
    uint32_t start = millis();
    while (millis() - start < timeout_ms) {
        float dist = ultrasonic_read_cm(sensor);
        if (dist < 0 || dist >= ULTRASONIC_TRIGGER_CM) {
            Serial.printf("[ULTRASONIC] %s sensor clear — %.1fcm\n",
                          sensor == SENSOR_ENTRY ? "Entry" : "Exit", dist);
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    Serial.printf("[ULTRASONIC] %s sensor timeout — force close\n",
                  sensor == SENSOR_ENTRY ? "Entry" : "Exit");
    return false;
}
