#include "buzzer.h"

void buzzer_init() {
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
    Serial.println("[BUZZER] Ready");
}

void buzzer_beep_granted() {
    digitalWrite(PIN_BUZZER, HIGH);
    vTaskDelay(pdMS_TO_TICKS(150));
    digitalWrite(PIN_BUZZER, LOW);
}

void buzzer_beep_denied() {
    for (int i = 0; i < 2; i++) {
        digitalWrite(PIN_BUZZER, HIGH);
        vTaskDelay(pdMS_TO_TICKS(100));
        digitalWrite(PIN_BUZZER, LOW);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void buzzer_beep_manual() {
    digitalWrite(PIN_BUZZER, HIGH);
    vTaskDelay(pdMS_TO_TICKS(400));
    digitalWrite(PIN_BUZZER, LOW);
}
