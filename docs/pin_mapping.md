# ESP32 Pin Mapping — v2 (Servo + Ultrasonic + LCD)

## RFID RC522 (SPI)
| RC522 Pin | ESP32 GPIO | Notes                    |
|-----------|------------|--------------------------|
| SDA (SS)  | 5          |                          |
| SCK       | 18         | SPI default              |
| MOSI      | 23         | SPI default              |
| MISO      | 19         | SPI default              |
| RST       | 4          | Moved from GPIO22 to free it for the LCD's I2C clock |
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

## LCD 16x2 with I2C backpack (PCF8574)
| LCD Pin | ESP32 GPIO | Notes                          |
|---------|------------|--------------------------------|
| SDA     | 21         | ESP32 I2C default              |
| SCL     | 22         | ESP32 I2C default              |
| VCC     | 5V         | Most PCF8574 backpacks run on 5V |
| GND     | GND        |                                |

> I2C address 0x27 (set in `main.cpp`). If the screen stays blank, try 0x3F.

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

> ⚠️ GPIO34–39 are input-only and have **no internal pull-up resistors**, so
> `INPUT_PULLUP` has no effect on these pins. Wire an external pull-up
> (e.g. 10 kΩ to 3.3V) on each button, or the inputs can float and trigger on their own.

## Power Rails
| Rail | Powers                                  |
|------|-----------------------------------------|
| 5V   | Servo motors, HC-SR04 sensors, LCD      |
| 3.3V | RFID RC522                              |
| USB  | ESP32 (development) or dedicated 5V reg |

## Removed from v1
| Component     | Reason                                           |
|---------------|--------------------------------------------------|
| Gear Motor    | Replaced by servo — no driver needed             |
| L298N Driver  | No longer needed — servo drives directly         |
| IR Sensor x2  | Replaced by HC-SR04 ultrasonic — distance-based  |
| OLED SSD1306  | Replaced by a 16x2 I2C LCD                       |
