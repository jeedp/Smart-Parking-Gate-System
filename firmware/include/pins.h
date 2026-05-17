#pragma once

// ─── RFID RC522 (SPI) ─────────────────────────────────────────────────────────
#define PIN_RFID_SS      5
#define PIN_RFID_RST     4
// SCK=18  MOSI=23  MISO=19  (ESP32 SPI defaults)

// ─── Servo Motors ─────────────────────────────────────────────────────────────
#define PIN_SERVO_ENTRY  13
#define PIN_SERVO_EXIT   14

// ─── Ultrasonic Sensors (HC-SR04) ─────────────────────────────────────────────
// NOTE: Avoid GPIO 27 on some ESP32 boards (flash conflict)
// Entry sensor — placed AFTER entry gate arm
#define PIN_ULTRASONIC_ENTRY_TRIG  25
#define PIN_ULTRASONIC_ENTRY_ECHO  26

// Exit sensor — placed BEFORE exit gate arm
#define PIN_ULTRASONIC_EXIT_TRIG   16
#define PIN_ULTRASONIC_EXIT_ECHO   17

// ─── LCD 16x2 I2C Backpack ────────────────────────────────────────────────────
#define PIN_LCD_SDA      21
#define PIN_LCD_SCL      22

// ─── Buzzer ───────────────────────────────────────────────────────────────────
#define PIN_BUZZER       32

// ─── Push Buttons (manual override) ──────────────────────────────────────────
// GPIO 34 and 35 are input-only — fine for buttons with external pull-up
#define PIN_BTN_OPEN     34
#define PIN_BTN_CLOSE    35
