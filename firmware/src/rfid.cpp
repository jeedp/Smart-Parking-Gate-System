#include "rfid.h"

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
    for (uint8_t i = 0; i < AUTHORIZED_COUNT; i++) {
        if (uid.equals(AUTHORIZED_UIDS[i])) return true;
    }
    return false;
}
