#pragma once

// ─── Wi-Fi ────────────────────────────────────────────────────────────────────
#define WIFI_SSID        "realme-narzo-50A-Prime"
#define WIFI_PASSWORD    "jedpadro123"

// ─── Firebase ─────────────────────────────────────────────────────────────────
#define API_KEY           "AIzaSyANEm_QXOkhhC6JlH6YqkuUgUkcChr_qCQ"
#define DATABASE_URL      "https://smart-parking-gate-system-default-rtdb.asia-southeast1.firebasedatabase.app"

#define USER_EMAIL        "jedpadro@gmail.com"
#define USER_PASSWORD     "jedpadro123"

// ─── Parking Settings ─────────────────────────────────────────────────────────
#define MAX_CAPACITY         12

// ─── Servo Angles ─────────────────────────────────────────────────────────────
#define SERVO_GATE_OPEN      90    // Degrees — gate arm raised horizontal
#define SERVO_GATE_CLOSED     0    // Degrees — gate arm down (blocking)

// ─── Ultrasonic Settings ──────────────────────────────────────────────────────
// Gate closes when ultrasonic reads a distance GREATER than this threshold —
// meaning the car has fully cleared the sensor and driven past it
#define ULTRASONIC_TRIGGER_CM   80    // Trigger threshold in centimeters
#define ULTRASONIC_TIMEOUT_US   30000 // pulseIn timeout (30ms ≈ ~5m max range)

// How long to wait for a car to clear the entry sensor before force-closing (ms)
#define GATE_CLEAR_TIMEOUT_MS   15000

// ─── Firebase push interval ───────────────────────────────────────────────────
#define FIREBASE_PUSH_MS     2000

// ─── BLE ──────────────────────────────────────────────────────────────────────
#define BLE_DEVICE_NAME         "ParkingGate"
#define BLE_SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BLE_GATE_CHAR_UUID      "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BLE_STATUS_CHAR_UUID    "cba1d466-344c-4be3-ab3f-189f80dd7518"
