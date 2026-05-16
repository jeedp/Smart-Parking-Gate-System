#pragma once
#include <Arduino.h>
#include <MFRC522.h>
#include "pins.h"

// Authorized UIDs are defined in rfid.cpp — add your cards there
void   rfid_init();
bool   rfid_card_present();
String rfid_read_uid();
bool   rfid_is_authorized(const String& uid);
