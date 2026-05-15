#pragma once
#include <Arduino.h>
#include "pins.h"
#include "config.h"

// Which sensor to read
enum SensorId { SENSOR_ENTRY, SENSOR_EXIT };

void     ultrasonic_init();

// Returns distance in cm. Returns -1 on timeout (out of range).
float    ultrasonic_read_cm(SensorId sensor);

// Returns true if a car is detected (distance < ULTRASONIC_TRIGGER_CM)
bool     ultrasonic_car_present(SensorId sensor);

// Blocks until the car clears (distance > threshold) or timeout_ms elapses.
// Returns true if car cleared, false if timed out.
bool     ultrasonic_wait_car_clear(SensorId sensor, uint32_t timeout_ms);
