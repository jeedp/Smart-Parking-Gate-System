# ESP32 Pin Mapping

## RFID RC522 (SPI)
| RC522 Pin | ESP32 Pin | GPIO |
|-----------|-----------|------|
| SDA (SS)  | D5        | 5    |
| SCK       | D18       | 18   |
| MOSI      | D23       | 23   |
| MISO      | D19       | 19   |
| RST       | D22       | 22   |
| GND       | GND       | —    |
| 3.3V      | 3V3       | —    |

## Motor Driver L298N
| L298N Pin | ESP32 Pin | GPIO | Notes               |
|-----------|-----------|------|---------------------|
| IN1       | D26       | 26   | Motor direction     |
| IN2       | D27       | 27   | Motor direction     |
| ENA       | D25       | 25   | PWM speed control   |
| 12V       | 12V PSU   | —    | Motor power rail    |
| GND       | GND (common) | — | Share ground        |
| 5V out    | —         | —    | Do not use for ESP32|

## IR Sensors (Digital module)
| Sensor      | ESP32 Pin | GPIO | Notes                    |
|-------------|-----------|------|--------------------------|
| Entry OUT   | —         | 34   | Input-only, no pull-up   |
| Exit  OUT   | —         | 35   | Input-only, no pull-up   |
| Both VCC    | 5V        | —    |                          |
| Both GND    | GND       | —    |                          |

## OLED Display SSD1306 0.96" (I2C)
| OLED Pin | ESP32 Pin | GPIO |
|----------|-----------|------|
| SDA      | D21       | 21   |
| SCL      | D22       | 22   |
| VCC      | 3V3       | —    |
| GND      | GND       | —    |

> ⚠️ GPIO22 is shared between RFID RST and OLED SCL.
> Hold RFID RST HIGH after init so it doesn't interfere with I2C.

## Buzzer
| Buzzer Pin | ESP32 Pin | GPIO |
|------------|-----------|------|
| +          | D32       | 32   |
| −          | GND       | —    |

## Push Buttons
| Button | ESP32 Pin | GPIO | Config             |
|--------|-----------|------|--------------------|
| OPEN   | D13       | 13   | INPUT_PULLUP, FALLING ISR |
| CLOSE  | D14       | 14   | INPUT_PULLUP, FALLING ISR |

## Power Supply
| Rail | Powers             |
|------|--------------------|
| 12V  | L298N motor supply |
| 5V   | IR sensors, OLED   |
| 3.3V | RFID RC522         |
| USB  | ESP32 (dev only)   |
