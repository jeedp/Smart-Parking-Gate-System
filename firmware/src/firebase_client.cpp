#include "firebase_client.h"
#include "config.h"
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <WiFi.h>

static FirebaseData   _fb_data;
static FirebaseConfig _fb_config;
static FirebaseAuth   _fb_auth;

void firebase_init() {
    _fb_config.api_key      = API_KEY;
    _fb_config.database_url = DATABASE_URL;

    _fb_auth.user.email    = USER_EMAIL;
    _fb_auth.user.password = USER_PASSWORD;

    _fb_config.token_status_callback = tokenStatusCallback;

    Firebase.begin(&_fb_config, &_fb_auth);
    Firebase.reconnectNetwork(true);

    // Fixed typo: 8129 → 8192. SSL needs at least 8192 for the TX buffer.
    _fb_data.setBSSLBufferSize(8192, 1024);
    _fb_data.setResponseSize(4096);

    // Block here until Firebase is actually ready before returning.
    // Without this the DashboardTask calls firebase_push() while SSL is still
    // handshaking, causing the "ssl engine not ready" flood.
    Serial.print("[FIREBASE] Waiting for ready");
    uint8_t attempts = 0;
    while (!Firebase.ready() && attempts < 30) {
        vTaskDelay(pdMS_TO_TICKS(500));
        Serial.print(".");
        attempts++;
    }
    if (Firebase.ready()) {
        Serial.println("\n[FIREBASE] Ready");
    } else {
        Serial.println("\n[FIREBASE] Timeout — will retry on next push");
    }
}

bool firebase_connected() {
    return Firebase.ready();
}

void firebase_push(const ParkingState& s) {
    if (!firebase_connected()) return;

    FirebaseJson json;
    json.set("gate_status",      s.gate_open ? "OPEN" : "CLOSED");
    json.set("spaces_available", s.spaces_available);
    json.set("total_entries",    s.total_entries);
    json.set("total_exits",      s.total_exits);
    json.set("last_card",        s.last_card);
    json.set("last_event",       s.last_event);
    json.set("updated_at/.sv",   "timestamp");

    if (!Firebase.RTDB.updateNode(&_fb_data, "/parking", &json)) {
        Serial.printf("[FIREBASE] Push failed: %s\n", _fb_data.errorReason().c_str());
    }
}

void firebase_log_event(const String& card_uid, const String& event) {
    if (!firebase_connected()) return;

    FirebaseJson entry;
    entry.set("card",     card_uid);
    entry.set("event",    event);
    entry.set("time/.sv", "timestamp");

    if (!Firebase.RTDB.push(&_fb_data, "/log", &entry)) {
        Serial.printf("[FIREBASE] Log failed: %s\n", _fb_data.errorReason().c_str());
    }
}
