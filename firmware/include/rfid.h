#pragma once
#include <Arduino.h>
#include <MFRC522.h>
#include "pins.h"

// List of authorized card UIDs (add your card UIDs here as hex strings)
const char* AUTHORIZED_UIDS[] = {
    "4A3FB21C",
    "7B2A09EE",
    "C3885D7A",
};
const uint8_t AUTHORIZED_COUNT = sizeof(AUTHORIZED_UIDS) / sizeof(AUTHORIZED_UIDS[0]);

void     rfid_init();
bool     rfid_card_present();
String   rfid_read_uid();
bool     rfid_is_authorized(const String& uid);
