#include "sensors.h"

SemaphoreHandle_t sem_entry = nullptr;
SemaphoreHandle_t sem_exit  = nullptr;

static unsigned long _last_entry_ms = 0;
static unsigned long _last_exit_ms  = 0;

void IRAM_ATTR isr_entry() {
    unsigned long now = millis();
    if (now - _last_entry_ms < IR_DEBOUNCE_MS) return;
    _last_entry_ms = now;
    BaseType_t woken = pdFALSE;
    xSemaphoreGiveFromISR(sem_entry, &woken);
    portYIELD_FROM_ISR(woken);
}

void IRAM_ATTR isr_exit() {
    unsigned long now = millis();
    if (now - _last_exit_ms < IR_DEBOUNCE_MS) return;
    _last_exit_ms = now;
    BaseType_t woken = pdFALSE;
    xSemaphoreGiveFromISR(sem_exit, &woken);
    portYIELD_FROM_ISR(woken);
}

void sensors_init() {
    // Create binary semaphores for ISR → task signalling
    sem_entry = xSemaphoreCreateBinary();
    sem_exit  = xSemaphoreCreateBinary();

    pinMode(PIN_IR_ENTRY, INPUT);
    pinMode(PIN_IR_EXIT,  INPUT);

    // Attach hardware interrupts on FALLING edge (beam broken = LOW)
    attachInterrupt(digitalPinToInterrupt(PIN_IR_ENTRY), isr_entry, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_IR_EXIT),  isr_exit,  FALLING);

    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);

    Serial.println("[SENSORS] IR interrupts armed, buzzer ready");
}

void buzzer_beep_granted() {
    // 1 short beep — access granted
    digitalWrite(PIN_BUZZER, HIGH);
    vTaskDelay(pdMS_TO_TICKS(150));
    digitalWrite(PIN_BUZZER, LOW);
}

void buzzer_beep_denied() {
    // 2 short beeps — access denied
    for (int i = 0; i < 2; i++) {
        digitalWrite(PIN_BUZZER, HIGH);
        vTaskDelay(pdMS_TO_TICKS(100));
        digitalWrite(PIN_BUZZER, LOW);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void buzzer_beep_manual() {
    // 1 long beep — manual override
    digitalWrite(PIN_BUZZER, HIGH);
    vTaskDelay(pdMS_TO_TICKS(400));
    digitalWrite(PIN_BUZZER, LOW);
}
