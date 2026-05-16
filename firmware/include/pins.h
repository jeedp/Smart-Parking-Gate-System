#pragma once

// ─── RFID RC522 (SPI) ─────────────────────────────────────────────────────────
#define PIN_RFID_SS      5
#define PIN_RFID_RST     4     // Moved from GPIO22 — freed for I2C SCL
// SCK=18  MOSI=23  MISO=19  (ESP32 SPI defaults)

// ─── Servo Motors ─────────────────────────────────────────────────────────────
// Power servo VCC from 5V rail (NOT 3.3V), share GND with ESP32
#define PIN_SERVO_ENTRY  13    // Entrance gate servo signal wire
#define PIN_SERVO_EXIT   14    // Exit gate servo signal wire

// ─── Ultrasonic Sensors (HC-SR04) ─────────────────────────────────────────────
// Entry: placed AFTER entrance gate — detects car has fully passed through
#define PIN_ULTRASONIC_ENTRY_TRIG  26
#define PIN_ULTRASONIC_ENTRY_ECHO  27

// Exit: placed BEFORE exit gate — detects car approaching to leave
#define PIN_ULTRASONIC_EXIT_TRIG   32
#define PIN_ULTRASONIC_EXIT_ECHO   33

// ─── LCD 16x2 I2C Backpack ────────────────────────────────────────────────────
// SDA and SCL are the ESP32 I2C defaults — no need to redefine in code
// LCD I2C address is typically 0x27 (PCF8574) — change in main.cpp if 0x3F
#define PIN_LCD_SDA      21
#define PIN_LCD_SCL      22

// ─── Buzzer ───────────────────────────────────────────────────────────────────
#define PIN_BUZZER       25

// ─── Push Buttons (manual override) ──────────────────────────────────────────
#define PIN_BTN_OPEN     34   // INPUT_PULLUP, FALLING edge ISR
#define PIN_BTN_CLOSE    35   // INPUT_PULLUP, FALLING edge ISR
