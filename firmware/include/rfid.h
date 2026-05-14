#pragma once
#include <Arduino.h>
#include <MFRC522.h>
#include "pins.h"

// List of authorized card UIDs (add your card UIDs here as hex strings)
extern const char* AUTHORIZED_UIDS[];
extern const uint8_t AUTHORIZED_COUNT;

void     rfid_init();
bool     rfid_card_present();
String   rfid_read_uid();
bool     rfid_is_authorized(const String& uid);
