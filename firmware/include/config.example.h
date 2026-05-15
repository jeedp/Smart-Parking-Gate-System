#pragma once

// ─── Wi-Fi ────────────────────────────────────────────────────────────────────
#define WIFI_SSID        "YOUR_WIFI_SSID"
#define WIFI_PASSWORD    "YOUR_WIFI_PASSWORD"

// ─── Firebase ─────────────────────────────────────────────────────────────────
#define FIREBASE_HOST    "your-project-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH    "YOUR_FIREBASE_DATABASE_SECRET"

// ─── Parking Settings ─────────────────────────────────────────────────────────
#define MAX_CAPACITY     12       // Total parking spaces
#define GATE_OPEN_MS     5000     // How long gate stays open (ms)
#define IR_DEBOUNCE_MS   200      // IR sensor debounce window (ms)
#define FIREBASE_PUSH_MS 2000     // How often to push data to Firebase (ms)

// ─── BLE ──────────────────────────────────────────────────────────────────────
#define BLE_DEVICE_NAME         "ParkingGate"
#define BLE_SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BLE_GATE_CHAR_UUID      "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BLE_STATUS_CHAR_UUID    "cba1d466-344c-4be3-ab3f-189f80dd7518"
