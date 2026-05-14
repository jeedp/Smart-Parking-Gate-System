#pragma once
#include <Arduino.h>

struct ParkingState {
    int  spaces_available;
    int  total_entries;
    int  total_exits;
    bool gate_open;
    String last_card;
    String last_event;   // "GRANTED" | "DENIED" | "MANUAL"
};

void   firebase_init();
bool   firebase_connected();
void   firebase_push(const ParkingState& state);
void   firebase_log_event(const String& card_uid, const String& event);
