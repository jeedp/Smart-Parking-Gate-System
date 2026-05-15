#pragma once

// ─── RFID RC522 (SPI) ─────────────────────────────────────────────────────────
#define PIN_RFID_SS      5
#define PIN_RFID_RST     22
// SCK=18  MOSI=23  MISO=19  (ESP32 SPI defaults)

// ─── Servo Motors ─────────────────────────────────────────────────────────────
// Each servo connects directly to ESP32 GPIO — no driver needed
// Power servo VCC from 5V rail (NOT 3.3V), share GND with ESP32
#define PIN_SERVO_ENTRY  13    // Entrance gate servo signal wire
#define PIN_SERVO_EXIT   14    // Exit gate servo signal wire

// ─── Ultrasonic Sensors (HC-SR04) ─────────────────────────────────────────────
// Entry ultrasonic: placed AFTER entrance gate — detects car has fully entered
#define PIN_ULTRASONIC_ENTRY_TRIG  26
#define PIN_ULTRASONIC_ENTRY_ECHO  27

// Exit ultrasonic: placed BEFORE exit gate — detects car approaching to exit
#define PIN_ULTRASONIC_EXIT_TRIG   32
#define PIN_ULTRASONIC_EXIT_ECHO   33

// ─── OLED Display (I2C) ───────────────────────────────────────────────────────
#define PIN_OLED_SDA     21
#define PIN_OLED_SCL     22   // Shared with RFID RST — hold RST HIGH after init

// ─── Buzzer ───────────────────────────────────────────────────────────────────
#define PIN_BUZZER       25

// ─── Push Buttons (manual override) ──────────────────────────────────────────
#define PIN_BTN_OPEN     34   // INPUT_PULLUP, FALLING edge ISR
#define PIN_BTN_CLOSE    35   // INPUT_PULLUP, FALLING edge ISR
