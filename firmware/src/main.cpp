#include <Arduino.h>
#include <WiFi.h>
#include <U8g2lib.h>
#include "config.h"
#include "pins.h"
#include "rfid.h"
#include "motor.h"
#include "sensors.h"
#include "firebase_client.h"
#include "ble_control.h"

// ─── OLED ─────────────────────────────────────────────────────────────────────
static U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

// ─── Shared state (protected by mutex) ────────────────────────────────────────
static SemaphoreHandle_t state_mutex;
static ParkingState g_state = {
    .spaces_available = MAX_CAPACITY,
    .total_entries    = 0,
    .total_exits      = 0,
    .gate_open        = false,
    .last_card        = "",
    .last_event       = "INIT"
};

// ─── Button state ─────────────────────────────────────────────────────────────
static volatile bool btn_open_pressed  = false;
static volatile bool btn_close_pressed = false;

void IRAM_ATTR isr_btn_open()  { btn_open_pressed  = true; }
void IRAM_ATTR isr_btn_close() { btn_close_pressed = true; }

// ─── OLED helper ──────────────────────────────────────────────────────────────
void oled_update(const ParkingState& s) {
    oled.clearBuffer();
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(0,  10, "SMART PARKING GATE");
    oled.drawHLine(0, 13, 128);
    oled.drawStr(0,  26, s.gate_open ? "Gate: OPEN" : "Gate: CLOSED");
    String spaces_str = "Spaces: " + String(s.spaces_available) + "/" + String(MAX_CAPACITY);
    oled.drawStr(0,  38, spaces_str.c_str());
    String event_str = "Last: " + s.last_event;
    oled.drawStr(0,  50, event_str.c_str());
    if (s.last_card.length() > 0) {
        oled.drawStr(0, 62, s.last_card.c_str());
    }
    oled.sendBuffer();
}

// ─── TASK 1: Gate control (Core 0) ────────────────────────────────────────────
// Handles: RFID auth, IR counting, button overrides, BLE commands, OLED
void task_gate_control(void*) {
    Serial.println("[TASK] GateControlTask started on core " + String(xPortGetCoreID()));

    rfid_init();
    sensors_init();
    motor_init();
    ble_init();
    oled.begin();
    oled_update(g_state);

    // Button interrupts
    pinMode(PIN_BTN_OPEN,  INPUT_PULLUP);
    pinMode(PIN_BTN_CLOSE, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_OPEN),  isr_btn_open,  FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_CLOSE), isr_btn_close, FALLING);

    for (;;) {
        // ── 1. Check RFID ──────────────────────────────────────────────────────
        if (rfid_card_present()) {
            String uid = rfid_read_uid();
            bool auth  = rfid_is_authorized(uid);
            Serial.printf("[RFID] Card: %s — %s\n", uid.c_str(), auth ? "GRANTED" : "DENIED");

            if (auth) {
                buzzer_beep_granted();
                motor_open_gate();
                xSemaphoreTake(state_mutex, portMAX_DELAY);
                g_state.gate_open  = true;
                g_state.last_card  = uid;
                g_state.last_event = "GRANTED";
                xSemaphoreGive(state_mutex);
                firebase_log_event(uid, "GRANTED");

                // Wait for vehicle to pass (IR entry semaphore, 10s timeout)
                if (xSemaphoreTake(sem_entry, pdMS_TO_TICKS(10000)) == pdTRUE) {
                    xSemaphoreTake(state_mutex, portMAX_DELAY);
                    g_state.total_entries++;
                    g_state.spaces_available = max(0, g_state.spaces_available - 1);
                    xSemaphoreGive(state_mutex);
                }
                motor_close_gate();
                xSemaphoreTake(state_mutex, portMAX_DELAY);
                g_state.gate_open = false;
                xSemaphoreGive(state_mutex);
            } else {
                buzzer_beep_denied();
                xSemaphoreTake(state_mutex, portMAX_DELAY);
                g_state.last_card  = uid;
                g_state.last_event = "DENIED";
                xSemaphoreGive(state_mutex);
                firebase_log_event(uid, "DENIED");
            }
        }

        // ── 2. Check exit IR sensor ────────────────────────────────────────────
        if (xSemaphoreTake(sem_exit, 0) == pdTRUE) {
            xSemaphoreTake(state_mutex, portMAX_DELAY);
            g_state.total_exits++;
            g_state.spaces_available = min(MAX_CAPACITY, g_state.spaces_available + 1);
            g_state.last_event = "EXIT";
            xSemaphoreGive(state_mutex);
            Serial.println("[IR] Vehicle exited");
        }

        // ── 3. Button overrides ────────────────────────────────────────────────
        if (btn_open_pressed) {
            btn_open_pressed = false;
            buzzer_beep_manual();
            motor_open_gate();
            xSemaphoreTake(state_mutex, portMAX_DELAY);
            g_state.gate_open  = true;
            g_state.last_event = "MANUAL";
            xSemaphoreGive(state_mutex);
        }
        if (btn_close_pressed) {
            btn_close_pressed = false;
            motor_close_gate();
            xSemaphoreTake(state_mutex, portMAX_DELAY);
            g_state.gate_open  = false;
            g_state.last_event = "MANUAL";
            xSemaphoreGive(state_mutex);
        }

        // ── 4. BLE commands ────────────────────────────────────────────────────
        BleCommand cmd = ble_get_command();
        if (cmd == BLE_OPEN && !motor_is_open()) {
            buzzer_beep_manual();
            motor_open_gate();
            xSemaphoreTake(state_mutex, portMAX_DELAY);
            g_state.gate_open  = true;
            g_state.last_event = "BLE";
            xSemaphoreGive(state_mutex);
        } else if (cmd == BLE_CLOSE && motor_is_open()) {
            motor_close_gate();
            xSemaphoreTake(state_mutex, portMAX_DELAY);
            g_state.gate_open  = false;
            g_state.last_event = "BLE";
            xSemaphoreGive(state_mutex);
        }

        // ── 5. Update OLED ─────────────────────────────────────────────────────
        xSemaphoreTake(state_mutex, portMAX_DELAY);
        ParkingState snap = g_state;
        xSemaphoreGive(state_mutex);
        oled_update(snap);
        ble_notify_status(snap.gate_open, snap.spaces_available);

        vTaskDelay(pdMS_TO_TICKS(100));   // Yield — feeds watchdog
    }
}

// ─── TASK 2: Dashboard / Firebase push (Core 1) ───────────────────────────────
// Handles: Wi-Fi connection, Firebase push every FIREBASE_PUSH_MS
void task_dashboard(void*) {
    Serial.println("[TASK] DashboardTask started on core " + String(xPortGetCoreID()));

    // Connect Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("[WIFI] Connecting");
    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(pdMS_TO_TICKS(500));
        Serial.print(".");
    }
    Serial.printf("\n[WIFI] Connected — IP: %s\n", WiFi.localIP().toString().c_str());

    firebase_init();

    TickType_t last_push = xTaskGetTickCount();

    for (;;) {
        if (xTaskGetTickCount() - last_push >= pdMS_TO_TICKS(FIREBASE_PUSH_MS)) {
            last_push = xTaskGetTickCount();

            // Reconnect Wi-Fi if dropped
            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("[WIFI] Reconnecting...");
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
    Serial.println("\n=== SMART PARKING GATE SYSTEM ===");

    state_mutex = xSemaphoreCreateMutex();

    // Pin GateControlTask to Core 0, DashboardTask to Core 1
    xTaskCreatePinnedToCore(task_gate_control, "GateControl", 8192, nullptr, 5, nullptr, 0);
    xTaskCreatePinnedToCore(task_dashboard,    "Dashboard",   8192, nullptr, 3, nullptr, 1);
}

// ─── loop() — unused (FreeRTOS tasks handle everything) ───────────────────────
void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }
