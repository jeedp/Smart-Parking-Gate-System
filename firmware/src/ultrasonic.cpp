#include "ultrasonic.h"

void ultrasonic_init() {
    // Entry sensor
    pinMode(PIN_ULTRASONIC_ENTRY_TRIG, OUTPUT);
    pinMode(PIN_ULTRASONIC_ENTRY_ECHO, INPUT);
    digitalWrite(PIN_ULTRASONIC_ENTRY_TRIG, LOW);

    // Exit sensor
    pinMode(PIN_ULTRASONIC_EXIT_TRIG, OUTPUT);
    pinMode(PIN_ULTRASONIC_EXIT_ECHO, INPUT);
    digitalWrite(PIN_ULTRASONIC_EXIT_TRIG, LOW);

    Serial.println("[ULTRASONIC] Entry and exit sensors ready");
}

float ultrasonic_read_cm(SensorId sensor) {
    uint8_t trig = (sensor == SENSOR_ENTRY) ? PIN_ULTRASONIC_ENTRY_TRIG : PIN_ULTRASONIC_EXIT_TRIG;
    uint8_t echo = (sensor == SENSOR_ENTRY) ? PIN_ULTRASONIC_ENTRY_ECHO : PIN_ULTRASONIC_EXIT_ECHO;

    // Send 10µs trigger pulse
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    // Measure echo pulse width
    long duration = pulseIn(echo, HIGH, ULTRASONIC_TIMEOUT_US);
    if (duration == 0) return -1.0f;   // Timeout — out of range

    // Convert to cm: sound travels 0.0343 cm/µs, divide by 2 for round-trip
    return (duration * 0.0343f) / 2.0f;
}

bool ultrasonic_car_present(SensorId sensor) {
    float dist = ultrasonic_read_cm(sensor);
    if (dist < 0) return false;              // Timeout = nothing detected
    return dist < ULTRASONIC_TRIGGER_CM;     // Car detected if closer than threshold
}

bool ultrasonic_wait_car_clear(SensorId sensor, uint32_t timeout_ms) {
    uint32_t start = millis();
    while (millis() - start < timeout_ms) {
        float dist = ultrasonic_read_cm(sensor);
        // Car has cleared if: reading timed out (nothing in range) OR distance > threshold
        if (dist < 0 || dist >= ULTRASONIC_TRIGGER_CM) {
            Serial.printf("[ULTRASONIC] Car cleared sensor %d — %.1f cm\n", sensor, dist);
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(100));   // Poll every 100ms
    }
    Serial.printf("[ULTRASONIC] Timeout waiting for car to clear sensor %d\n", sensor);
    return false;
}
