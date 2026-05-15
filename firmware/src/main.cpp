#include <Arduino.h>
#include <WiFi.h>
#include <U8g2lib.h>
#include "config.h"
#include "pins.h"
#include "rfid.h"
#include "gate.h"
#include "sensors.h"
#include "ultrasonic.h"
#include "firebase_client.h"
#include "ble_control.h"

// ─── OLED ─────────────────────────────────────────────────────────────────────
static U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

// ─── Shared state ─────────────────────────────────────────────────────────────
static SemaphoreHandle_t state_mutex;
static ParkingState g_state = {
    .spaces_available = MAX_CAPACITY,
    .total_entries    = 0,
    .total_exits      = 0,
    .gate_open        = false,
    .last_card        = "",
    .last_event       = "INIT"
};

// ─── Button ISRs ──────────────────────────────────────────────────────────────
static volatile bool btn_open_pressed  = false;
static volatile bool btn_close_pressed = false;
void IRAM_ATTR isr_btn_open()  { btn_open_pressed  = true; }
void IRAM_ATTR isr_btn_close() { btn_close_pressed = true; }

// ─── OLED helper ──────────────────────────────────────────────────────────────
void oled_update(const ParkingState& s) {
    oled.clearBuffer();
    oled.setFont(u8g2_font_6x10_tf);

    oled.drawStr(0, 10, "SMART PARKING GATE");
    oled.drawHLine(0, 13, 128);

    String entry_str = "Entry: " + String(gate_is_open(GATE_ENTRY) ? "OPEN " : "CLOSED");
    String exit_str  = "Exit:  " + String(gate_is_open(GATE_EXIT)  ? "OPEN " : "CLOSED");
    oled.drawStr(0, 26, entry_str.c_str());
    oled.drawStr(0, 38, exit_str.c_str());

    String spaces_str = "Spaces: " + String(s.spaces_available) + "/" + String(MAX_CAPACITY);
    oled.drawStr(0, 50, spaces_str.c_str());
    oled.drawStr(0, 62, s.last_event.c_str());

    oled.sendBuffer();
}

// ─── TASK 1: Gate Control (Core 0) ────────────────────────────────────────────
// Handles: RFID auth → entry gate, ultrasonic close trigger,
//          exit ultrasonic → exit gate, button overrides, BLE commands
void task_gate_control(void*) {
    Serial.println("[TASK] GateControlTask on core " + String(xPortGetCoreID()));

    rfid_init();
    gate_init();
    ultrasonic_init();
    ble_init();
    oled.begin();
    oled_update(g_state);

    pinMode(PIN_BTN_OPEN,  INPUT_PULLUP);
    pinMode(PIN_BTN_CLOSE, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_OPEN),  isr_btn_open,  FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_CLOSE), isr_btn_close, FALLING);

    for (;;) {

        // ── 1. RFID scan → open entry gate ────────────────────────────────────
        if (rfid_card_present()) {
            String uid  = rfid_read_uid();
            bool   auth = rfid_is_authorized(uid);
            Serial.printf("[RFID] Card: %s — %s\n", uid.c_str(), auth ? "GRANTED" : "DENIED");

            xSemaphoreTake(state_mutex, portMAX_DELAY);
            g_state.last_card  = uid;
            g_state.last_event = auth ? "ENTRY GRANTED" : "ENTRY DENIED";
            xSemaphoreGive(state_mutex);

            if (auth) {
                buzzer_beep_granted();
                gate_open(GATE_ENTRY);

                xSemaphoreTake(state_mutex, portMAX_DELAY);
                g_state.gate_open = true;
                xSemaphoreGive(state_mutex);

                firebase_log_event(uid, "GRANTED");

                // ── 2. Wait for car to pass entry ultrasonic then close ────────
                // Ultrasonic is placed AFTER the gate arm.
                // First wait for car to appear (come within range)...
                Serial.println("[ENTRY] Waiting for car to appear under sensor...");
                uint32_t appear_start = millis();
                while (!ultrasonic_car_present(SENSOR_ENTRY)) {
                    if (millis() - appear_start > GATE_CLEAR_TIMEOUT_MS) break;
                    vTaskDelay(pdMS_TO_TICKS(100));
                }

                // ...then wait for it to fully clear (drive past sensor)
                Serial.println("[ENTRY] Car detected — waiting to clear...");
                bool cleared = ultrasonic_wait_car_clear(SENSOR_ENTRY, GATE_CLEAR_TIMEOUT_MS);

                gate_close(GATE_ENTRY);

                xSemaphoreTake(state_mutex, portMAX_DELAY);
                g_state.gate_open = false;
                g_state.total_entries++;
                g_state.spaces_available = max(0, g_state.spaces_available - 1);
                g_state.last_event = cleared ? "ENTERED" : "TIMEOUT CLOSE";
                xSemaphoreGive(state_mutex);

            } else {
                buzzer_beep_denied();
                firebase_log_event(uid, "DENIED");
            }
        }

        // ── 3. Exit ultrasonic → open exit gate automatically ─────────────────
        // Car approaches exit gate → sensor detects it → gate opens automatically
        // No RFID needed for exit
        if (!gate_is_open(GATE_EXIT) && ultrasonic_car_present(SENSOR_EXIT)) {
            Serial.println("[EXIT] Car detected at exit — opening exit gate");
            buzzer_beep_granted();
            gate_open(GATE_EXIT);

            xSemaphoreTake(state_mutex, portMAX_DELAY);
            g_state.last_event = "EXIT DETECT";
            xSemaphoreGive(state_mutex);

            // Wait for car to fully clear, then close
            bool cleared = ultrasonic_wait_car_clear(SENSOR_EXIT, GATE_CLEAR_TIMEOUT_MS);
            gate_close(GATE_EXIT);

            xSemaphoreTake(state_mutex, portMAX_DELAY);
            g_state.total_exits++;
            g_state.spaces_available = min(MAX_CAPACITY, g_state.spaces_available + 1);
            g_state.last_event = cleared ? "EXITED" : "EXIT TIMEOUT";
            xSemaphoreGive(state_mutex);

            firebase_log_event("—", "EXIT");
        }

        // ── 4. Manual button overrides ────────────────────────────────────────
        if (btn_open_pressed) {
            btn_open_pressed = false;
            buzzer_beep_manual();
            gate_open(GATE_ENTRY);
            xSemaphoreTake(state_mutex, portMAX_DELAY);
            g_state.gate_open  = true;
            g_state.last_event = "MANUAL OPEN";
            xSemaphoreGive(state_mutex);
        }
        if (btn_close_pressed) {
            btn_close_pressed = false;
            gate_close(GATE_ENTRY);
            gate_close(GATE_EXIT);
            xSemaphoreTake(state_mutex, portMAX_DELAY);
            g_state.gate_open  = false;
            g_state.last_event = "MANUAL CLOSE";
            xSemaphoreGive(state_mutex);
        }

        // ── 5. BLE commands ───────────────────────────────────────────────────
        BleCommand cmd = ble_get_command();
        if (cmd == BLE_OPEN) {
            buzzer_beep_manual();
            gate_open(GATE_ENTRY);
            gate_open(GATE_EXIT);
            xSemaphoreTake(state_mutex, portMAX_DELAY);
            g_state.gate_open = true; g_state.last_event = "BLE OPEN";
            xSemaphoreGive(state_mutex);
        } else if (cmd == BLE_CLOSE) {
            gate_close(GATE_ENTRY);
            gate_close(GATE_EXIT);
            xSemaphoreTake(state_mutex, portMAX_DELAY);
            g_state.gate_open = false; g_state.last_event = "BLE CLOSE";
            xSemaphoreGive(state_mutex);
        }

        // ── 6. Update OLED + BLE notify ───────────────────────────────────────
        xSemaphoreTake(state_mutex, portMAX_DELAY);
        ParkingState snap = g_state;
        xSemaphoreGive(state_mutex);

        oled_update(snap);
        ble_notify_status(snap.gate_open, snap.spaces_available);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ─── TASK 2: Dashboard / Firebase push (Core 1) ───────────────────────────────
void task_dashboard(void*) {
    Serial.println("[TASK] DashboardTask on core " + String(xPortGetCoreID()));

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("[WIFI] Connecting");
    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(pdMS_TO_TICKS(500));
        Serial.print(".");
    }
    Serial.printf("\n[WIFI] IP: %s\n", WiFi.localIP().toString().c_str());

    firebase_init();
    TickType_t last_push = xTaskGetTickCount();

    for (;;) {
        if (xTaskGetTickCount() - last_push >= pdMS_TO_TICKS(FIREBASE_PUSH_MS)) {
            last_push = xTaskGetTickCount();

            if (WiFi.status() != WL_CONNECTED) {
                WiFi.reconnect();
                vTaskDelay(pdMS_TO_TICKS(2000));
                continue;
            }

            xSemaphoreTake(state_mutex, portMAX_DELAY);
            ParkingState snap = g_state;
            xSemaphoreGive(state_mutex);

            firebase_push(snap);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ─── setup() ──────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    Serial.println("\n=== SMART PARKING GATE SYSTEM v2 ===");
    Serial.println("Sensors: HC-SR04 Ultrasonic x2");
    Serial.println("Gates:   Servo Motor x2 (entry + exit)");

    state_mutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(task_gate_control, "GateControl", 8192, nullptr, 5, nullptr, 0);
    xTaskCreatePinnedToCore(task_dashboard,    "Dashboard",   8192, nullptr, 3, nullptr, 1);
}

void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }
