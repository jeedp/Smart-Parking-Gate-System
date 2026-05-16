#pragma once
#include <Arduino.h>
#include "pins.h"

void buzzer_init();
void buzzer_beep_granted();   // 1 short beep  — RFID access granted
void buzzer_beep_denied();    // 2 short beeps — RFID access denied
void buzzer_beep_manual();    // 1 long  beep  — manual / BLE override
