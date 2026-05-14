#pragma once

// ─── RFID RC522 (SPI) ─────────────────────────────────────────────────────────
#define PIN_RFID_SS      5
#define PIN_RFID_RST     22
// SCK=18  MOSI=23  MISO=19  (ESP32 SPI defaults — no need to define)

// ─── Motor Driver L298N ───────────────────────────────────────────────────────
#define PIN_MOTOR_IN1    26
#define PIN_MOTOR_IN2    27
#define PIN_MOTOR_ENA    25    // PWM-capable pin

// ─── IR Sensors ───────────────────────────────────────────────────────────────
#define PIN_IR_ENTRY     34    // Input-only GPIO (no pull-up needed with digital IR module)
#define PIN_IR_EXIT      35    // Input-only GPIO

// ─── OLED Display (I2C) ───────────────────────────────────────────────────────
#define PIN_OLED_SDA     21
#define PIN_OLED_SCL     22
// Note: SCL shares GPIO22 with RFID RST — ensure RST is held HIGH after init

// ─── Buzzer ───────────────────────────────────────────────────────────────────
#define PIN_BUZZER       32

// ─── Push Buttons ─────────────────────────────────────────────────────────────
#define PIN_BTN_OPEN     13
#define PIN_BTN_CLOSE    14
