#pragma once
#include <Arduino.h>
#include "pins.h"
#include "config.h"

// Semaphores for ISR → task communication (defined in sensors.cpp)
extern SemaphoreHandle_t sem_entry;
extern SemaphoreHandle_t sem_exit;

void sensors_init();
void buzzer_beep_granted();   // 1 short beep
void buzzer_beep_denied();    // 2 short beeps
void buzzer_beep_manual();    // 1 long beep

// ISR handlers — must be in IRAM
void IRAM_ATTR isr_entry();
void IRAM_ATTR isr_exit();
