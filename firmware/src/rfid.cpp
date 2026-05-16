#include "rfid.h"

// ─── Authorized card UIDs ─────────────────────────────────────────────────────
// Add or remove UIDs here. Format: uppercase hex, no separators.
static const char* AUTHORIZED_UIDS[] = {
    "4A3FB21C",
    "7B2A09EE",
    "C3885D7A",
};
static const uint8_t AUTHORIZED_COUNT = sizeof(AUTHORIZED_UIDS) / sizeof(AUTHORIZED_UIDS[0]);

static MFRC522 mfrc522(PIN_RFID_SS, PIN_RFID_RST);

void rfid_init() {
    SPI.begin();
    mfrc522.PCD_Init();
    mfrc522.PCD_DumpVersionToSerial();
    Serial.println("[RFID] Ready");
}

bool rfid_card_present() {
    return mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial();
}

String rfid_read_uid() {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
        uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return uid;
}

bool rfid_is_authorized(const String& uid) {
    // uid is already uppercased by rfid_read_uid()
    for (uint8_t i = 0; i < AUTHORIZED_COUNT; i++) {
        if (uid == String(AUTHORIZED_UIDS[i])) return true;
    }
    return false;
}
