#include "firebase_client.h"
#include "config.h"
#include <FirebaseESP32.h>
#include <WiFi.h>

static FirebaseData   _fb_data;
static FirebaseConfig _fb_config;
static FirebaseAuth   _fb_auth;

void firebase_init() {
    _fb_config.host                          = FIREBASE_HOST;
    _fb_config.signer.tokens.legacy_token    = FIREBASE_AUTH;
    Firebase.begin(&_fb_config, &_fb_auth);
    Firebase.reconnectWiFi(true);
    Serial.println("[FIREBASE] Init done");
}

bool firebase_connected() {
    return Firebase.ready();
}

void firebase_push(const ParkingState& s) {
    if (!firebase_connected()) return;

    FirebaseJson json;
    json.set("gate_status",       s.gate_open ? "OPEN" : "CLOSED");
    json.set("spaces_available",  s.spaces_available);
    json.set("total_entries",     s.total_entries);
    json.set("total_exits",       s.total_exits);
    json.set("last_card",         s.last_card);
    json.set("last_event",        s.last_event);
    json.set("updated_at/.sv",    "timestamp");   // Firebase server timestamp

    if (!Firebase.updateNode(_fb_data, "/parking", json)) {
        Serial.printf("[FIREBASE] Push failed: %s\n", _fb_data.errorReason().c_str());
    }
}

void firebase_log_event(const String& card_uid, const String& event) {
    if (!firebase_connected()) return;

    FirebaseJson entry;
    entry.set("card",     card_uid);
    entry.set("event",    event);
    entry.set("time/.sv", "timestamp");

    if (!Firebase.pushJSON(_fb_data, "/log", entry)) {
        Serial.printf("[FIREBASE] Log failed: %s\n", _fb_data.errorReason().c_str());
    }
}
