#include <Arduino.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"
#include "pins.h"
#include "rfid.h"
#include "gate.h"
#include "ultrasonic.h"
#include "buzzer.h"
#include "firebase_client.h"
#include "ble_control.h"

// ─── LCD 16x2 I2C ─────────────────────────────────────────────────────────────
static LiquidCrystal_I2C lcd(0x27, 16, 2);

// ─── Shared state (mutex-protected) ───────────────────────────────────────────
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

// ─── LCD helpers ──────────────────────────────────────────────────────────────
static String lcd_fit(const String& s, uint8_t width = 16) {
    if (s.length() >= width) return s.substring(0, width);
    String out = s;
    while (out.length() < width) out += ' ';
    return out;
}

static void lcd_row(uint8_t row, const String& text) {
    lcd.setCursor(0, row);
    lcd.print(lcd_fit(text, 16));
}

// ─── LCD screen states ────────────────────────────────────────────────────────
void lcd_show_idle(const ParkingState& s) {
    String row0 = "PARK AVAIL: " + String(s.spaces_available) + "/" + String(MAX_CAPACITY);
    String in_s = gate_is_open(GATE_ENTRY) ? "OPEN " : "CLOSED ";
    lcd_row(0, row0);
    lcd_row(1, " Gate: " + in_s);
}

void lcd_show_granted(const String& uid) {
    lcd_row(0, "ACCESS GRANTED  ");
    String short_uid = uid.length() >= 8 ? uid.substring(uid.length() - 8) : uid;
    lcd_row(1, "UID: " + short_uid);
}

void lcd_show_denied(const String& uid) {
    lcd_row(0, "ACCESS DENIED   ");
    String short_uid = uid.length() >= 8 ? uid.substring(uid.length() - 8) : uid;
    lcd_row(1, "UID: " + short_uid);
}

void lcd_show_vehicle_event(const String& msg, int spaces) {
    lcd_row(0, msg);
    lcd_row(1, "Free: " + String(spaces) + "/" + String(MAX_CAPACITY) + " spaces");
}

void lcd_show_manual(const String& action) {
    lcd_row(0, "MANUAL OVERRIDE ");
    lcd_row(1, action);
}

void lcd_show_startup() {
    lcd_row(0, " SMART PARKING  ");
    lcd_row(1, "  GATE SYSTEM   ");
}

void lcd_show_wifi_connecting() {
    lcd_row(0, "WiFi connecting.");
    lcd_row(1, "Please wait...  ");
}

void lcd_show_wifi_ready(const String& ip) {
    lcd_row(0, "WiFi connected! ");
    lcd_row(1, ip.length() > 16 ? ip.substring(ip.length() - 16) : ip);
}

// ─── TASK 1: Gate Control (Core 0, priority 5) ────────────────────────────────
void task_gate_control(void*) {
    Serial.println("[TASK] GateControlTask on core " + String(xPortGetCoreID()));

    lcd.init();
    lcd.backlight();
    lcd_show_startup();
    vTaskDelay(pdMS_TO_TICKS(1500));

    rfid_init();
    gate_init();
    ultrasonic_init();
    buzzer_init();
    ble_init();

    xSemaphoreTake(state_mutex, portMAX_DELAY);
    ParkingState snap = g_state;
    xSemaphoreGive(state_mutex);
    lcd_show_idle(snap);

    pinMode(PIN_BTN_OPEN,  INPUT_PULLUP);
    pinMode(PIN_BTN_CLOSE, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_OPEN),  isr_btn_open,  FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_CLOSE), isr_btn_close, FALLING);

    for (;;) {

        // ── 1. RFID scan → entry gate ──────────────────────────────────────────
        if (rfid_card_present()) {
            String uid  = rfid_read_uid();
            bool   auth = rfid_is_authorized(uid);
            Serial.printf("[RFID] UID: %s — %s\n", uid.c_str(), auth ? "GRANTED" : "DENIED");

            if (auth) {
                buzzer_beep_granted();
                lcd_show_granted(uid);
                gate_open(GATE_ENTRY);

                xSemaphoreTake(state_mutex, portMAX_DELAY);
                g_state.gate_open  = true;
                g_state.last_card  = uid;
                g_state.last_event = "ENTRY GRANTED";
                xSemaphoreGive(state_mutex);

                firebase_log_event(uid, "GRANTED");

                Serial.println("[ENTRY] Waiting for car...");
                uint32_t t0 = millis();
                while (!ultrasonic_car_present(SENSOR_ENTRY)) {
                    if (millis() - t0 > GATE_CLEAR_TIMEOUT_MS) {
                        Serial.println("[ENTRY] No car detected — force closing");
                        break;
                    }
                    vTaskDelay(pdMS_TO_TICKS(100));
                }

                lcd_row(0, "Wait: car clear ");
                lcd_row(1, "Gate stays open ");
                bool cleared = ultrasonic_wait_car_clear(SENSOR_ENTRY, GATE_CLEAR_TIMEOUT_MS);
                gate_close(GATE_ENTRY);

                xSemaphoreTake(state_mutex, portMAX_DELAY);
                g_state.gate_open        = false;
                g_state.total_entries++;
                g_state.spaces_available = max(0, g_state.spaces_available - 1);
                g_state.last_event       = cleared ? "ENTERED" : "TIMEOUT CLOSE";
                snap = g_state;
                xSemaphoreGive(state_mutex);

                lcd_show_vehicle_event("VEHICLE ENTERED ", snap.spaces_available);
                vTaskDelay(pdMS_TO_TICKS(2000));

            } else {
                buzzer_beep_denied();
                lcd_show_denied(uid);

                xSemaphoreTake(state_mutex, portMAX_DELAY);
                g_state.last_card  = uid;
                g_state.last_event = "ENTRY DENIED";
                xSemaphoreGive(state_mutex);

                firebase_log_event(uid, "DENIED");
                vTaskDelay(pdMS_TO_TICKS(2000));
            }
        }

        // ── 2. Exit ultrasonic ─────────────────────────────────────────────────
        if (!gate_is_open(GATE_EXIT) && ultrasonic_car_present(SENSOR_EXIT)) {
            Serial.println("[EXIT] Car exiting parking lot");
            buzzer_beep_granted();
            gate_open(GATE_EXIT);

            lcd_row(0, "  CAR EXITING   ");
            lcd_row(1, "  PARKING LOT   ");

            xSemaphoreTake(state_mutex, portMAX_DELAY);
            g_state.last_event = "EXIT DETECT";
            xSemaphoreGive(state_mutex);

            bool cleared = ultrasonic_wait_car_clear(SENSOR_EXIT, GATE_CLEAR_TIMEOUT_MS);
            gate_close(GATE_EXIT);

            xSemaphoreTake(state_mutex, portMAX_DELAY);
            g_state.total_exits++;
            g_state.spaces_available = min(MAX_CAPACITY, g_state.spaces_available + 1);
            g_state.last_event       = cleared ? "EXITED" : "EXIT TIMEOUT";
            snap = g_state;
            xSemaphoreGive(state_mutex);

            firebase_log_event("—", "EXIT");
            lcd_show_vehicle_event("VEHICLE EXITED  ", snap.spaces_available);
            vTaskDelay(pdMS_TO_TICKS(2000));
        }

        // ── 3. Physical button overrides ───────────────────────────────────────
        if (btn_open_pressed) {
            btn_open_pressed = false;
            buzzer_beep_manual();
            gate_open(GATE_ENTRY);
            lcd_show_manual("Gate: OPEN      ");

            xSemaphoreTake(state_mutex, portMAX_DELAY);
            g_state.gate_open  = true;
            g_state.last_event = "MANUAL OPEN";
            xSemaphoreGive(state_mutex);

            vTaskDelay(pdMS_TO_TICKS(1500));
        }
        if (btn_close_pressed) {
            btn_close_pressed = false;
            gate_close(GATE_ENTRY);
            gate_close(GATE_EXIT);
            lcd_show_manual("Gate: CLOSED    ");

            xSemaphoreTake(state_mutex, portMAX_DELAY);
            g_state.gate_open  = false;
            g_state.last_event = "MANUAL CLOSE";
            xSemaphoreGive(state_mutex);

            vTaskDelay(pdMS_TO_TICKS(1500));
        }

        // ── 4. BLE commands ────────────────────────────────────────────────────
        BleCommand cmd = ble_get_command();
        if (cmd == BLE_OPEN) {
            buzzer_beep_manual();
            gate_open(GATE_ENTRY);
            gate_open(GATE_EXIT);

            xSemaphoreTake(state_mutex, portMAX_DELAY);
            g_state.gate_open  = true;
            g_state.last_event = "BLE OPEN";
            snap = g_state;
            xSemaphoreGive(state_mutex);

            lcd_show_manual("BLE: OPEN       ");
            ble_notify_status(snap.gate_open, snap.spaces_available);
            vTaskDelay(pdMS_TO_TICKS(1500));

        } else if (cmd == BLE_CLOSE) {
            gate_close(GATE_ENTRY);
            gate_close(GATE_EXIT);

            xSemaphoreTake(state_mutex, portMAX_DELAY);
            g_state.gate_open  = false;
            g_state.last_event = "BLE CLOSE";
            snap = g_state;
            xSemaphoreGive(state_mutex);

            lcd_show_manual("BLE: CLOSED     ");
            ble_notify_status(snap.gate_open, snap.spaces_available);
            vTaskDelay(pdMS_TO_TICKS(1500));

        } else if (cmd == BLE_DISCONNECT) {
            Serial.println("[BLE] Disconnect requested by client");
            ble_disconnect_client();
        }

        // ── 5. Idle screen + BLE notify ───────────────────────────────────────
        xSemaphoreTake(state_mutex, portMAX_DELAY);
        snap = g_state;
        xSemaphoreGive(state_mutex);

        lcd_show_idle(snap);
        ble_notify_status(snap.gate_open, snap.spaces_available);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ─── TASK 2: Dashboard / Firebase push (Core 1, priority 3) ──────────────────
void task_dashboard(void*) {
    Serial.println("[TASK] DashboardTask on core " + String(xPortGetCoreID()));

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    xSemaphoreTake(state_mutex, portMAX_DELAY);
    lcd_show_wifi_connecting();
    xSemaphoreGive(state_mutex);

    Serial.print("[WIFI] Connecting");
    uint8_t retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 30) {
        vTaskDelay(pdMS_TO_TICKS(500));
        Serial.print(".");
        retries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        String ip = WiFi.localIP().toString();
        Serial.printf("\n[WIFI] Connected — IP: %s\n", ip.c_str());
        xSemaphoreTake(state_mutex, portMAX_DELAY);
        lcd_show_wifi_ready(ip);
        xSemaphoreGive(state_mutex);
        vTaskDelay(pdMS_TO_TICKS(2000));
    } else {
        Serial.println("\n[WIFI] Failed — dashboard offline");
    }

    firebase_init();   // Blocks until Firebase.ready() — no more premature pushes

    TickType_t last_push    = xTaskGetTickCount();
    uint8_t    fail_streak  = 0;  // Track consecutive push failures for backoff

    for (;;) {
        if (xTaskGetTickCount() - last_push >= pdMS_TO_TICKS(FIREBASE_PUSH_MS)) {
            last_push = xTaskGetTickCount();

            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("[WIFI] Lost connection — reconnecting");
                WiFi.reconnect();
                // Back off 5s on Wi-Fi loss — don't hammer reconnect
                vTaskDelay(pdMS_TO_TICKS(5000));
                continue;
            }

            if (!firebase_connected()) {
                fail_streak++;
                // Exponential-ish backoff: 2s, 4s, 8s, cap at 30s
                uint32_t wait_ms = min((uint32_t)(2000 * fail_streak), (uint32_t)30000);
                Serial.printf("[FIREBASE] Not ready — waiting %ums before retry\n", wait_ms);
                vTaskDelay(pdMS_TO_TICKS(wait_ms));
                continue;
            }

            fail_streak = 0;  // Reset on successful connection check

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
    Serial.println("Display: LCD 16x2 I2C (0x27)");
    Serial.println("Gates:   Servo x2 (entry GPIO13, exit GPIO14)");
    Serial.println("Sensors: HC-SR04 Ultrasonic x2");
    Serial.println("Auth:    RFID RC522");

    state_mutex = xSemaphoreCreateMutex();
    configASSERT(state_mutex);

    xTaskCreatePinnedToCore(task_gate_control, "GateControl", 8192,  nullptr, 5, nullptr, 0);
    // Stack increased from 8192 → 16384: Firebase SSL/TLS needs significantly
    // more stack than plain HTTP. 8192 caused the stack canary crash.
    xTaskCreatePinnedToCore(task_dashboard,    "Dashboard",   16384, nullptr, 3, nullptr, 1);
}

void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }
