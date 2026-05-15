# ESP32 Pin Mapping — v2 (Servo + Ultrasonic)

## RFID RC522 (SPI)
| RC522 Pin | ESP32 GPIO | Notes                    |
|-----------|------------|--------------------------|
| SDA (SS)  | 5          |                          |
| SCK       | 18         | SPI default              |
| MOSI      | 23         | SPI default              |
| MISO      | 19         | SPI default              |
| RST       | 22         | Shared with OLED SCL*    |
| VCC       | 3.3V       | NOT 5V                   |
| GND       | GND        |                          |

## Servo Motors (x2)
| Servo         | ESP32 GPIO | Power  | Notes                        |
|---------------|------------|--------|------------------------------|
| Entry gate    | 13         | 5V     | Signal only on GPIO          |
| Exit gate     | 14         | 5V     | Signal only on GPIO          |

> ⚠️ Power servos from the 5V rail directly — NOT from ESP32 3.3V pin.
> Share GND between servo, ESP32, and power supply.
> 0° = gate closed (arm down), 90° = gate open (arm raised).

## Ultrasonic Sensors HC-SR04 (x2)
| Sensor        | TRIG GPIO | ECHO GPIO | Power | Placement                        |
|---------------|-----------|-----------|-------|----------------------------------|
| Entry sensor  | 26        | 27        | 5V    | After entry gate arm — detects car clearing |
| Exit sensor   | 32        | 33        | 5V    | Before exit gate arm — detects car approaching |

> Trigger threshold: car detected if distance < 80 cm (set in config.h).
> Entry flow: RFID opens gate → car passes → ultrasonic detects clear → gate closes.
> Exit flow: car approaches exit sensor → gate opens automatically → car clears → gate closes.

## OLED Display SSD1306 0.96" (I2C)
| OLED Pin | ESP32 GPIO | Notes                  |
|----------|------------|------------------------|
| SDA      | 21         |                        |
| SCL      | 22         | Shared with RFID RST*  |
| VCC      | 3.3V       |                        |
| GND      | GND        |                        |

> *GPIO22 shared: hold RFID RST HIGH after rfid_init() so it acts as a
> passive pin and does not interfere with I2C clock.

## Buzzer
| Pin | ESP32 GPIO |
|-----|------------|
| +   | 25         |
| −   | GND        |

## Push Buttons (Manual Override)
| Button | ESP32 GPIO | Config                     |
|--------|------------|----------------------------|
| OPEN   | 34         | INPUT_PULLUP, FALLING ISR  |
| CLOSE  | 35         | INPUT_PULLUP, FALLING ISR  |

## Power Rails
| Rail | Powers                                  |
|------|-----------------------------------------|
| 5V   | Servo motors, HC-SR04 sensors, OLED     |
| 3.3V | RFID RC522                              |
| USB  | ESP32 (development) or dedicated 5V reg |

## Removed from v1
| Component     | Reason                                           |
|---------------|--------------------------------------------------|
| Gear Motor    | Replaced by servo — no driver needed             |
| L298N Driver  | No longer needed — servo drives directly         |
| IR Sensor x2  | Replaced by HC-SR04 ultrasonic — distance-based  |
