#include "ble_control.h"
#include "config.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

static BLECharacteristic* _gate_char    = nullptr;
static BLECharacteristic* _status_char  = nullptr;
static volatile BleCommand _pending     = BLE_NONE;
static bool _client_connected           = false;
static BLEServer* _server               = nullptr;

// ─── Server callbacks ─────────────────────────────────────────────────────────
class ServerCB : public BLEServerCallbacks {
    void onConnect(BLEServer*) override {
        _client_connected = true;
        Serial.println("[BLE] Client connected");
    }
    void onDisconnect(BLEServer*) override {
        _client_connected = false;
        Serial.println("[BLE] Client disconnected — restarting advertising");
        // Restart advertising so new clients can connect
        BLEDevice::startAdvertising();
    }
};

// ─── Gate characteristic write callback ───────────────────────────────────────
class GateCB : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* c) override {
        // getValue() returns std::string in newer cores — convert safely
        std::string raw = c->getValue();
        String val(raw.c_str());
        val.toUpperCase();
        val.trim();

        if      (val == "OPEN")    _pending = BLE_OPEN;
        else if (val == "CLOSE")   _pending = BLE_CLOSE;
        else if (val == "REFRESH") _pending = BLE_REFRESH;
        else Serial.printf("[BLE] Unknown command: %s\n", val.c_str());

        Serial.printf("[BLE] Command received: %s\n", val.c_str());
    }
};

void ble_init() {
    BLEDevice::init(BLE_DEVICE_NAME);
    _server = BLEDevice::createServer();
    _server->setCallbacks(new ServerCB());

    BLEService* service = _server->createService(BLE_SERVICE_UUID);

    // Gate command characteristic (Write only)
    _gate_char = service->createCharacteristic(
        BLE_GATE_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    _gate_char->setCallbacks(new GateCB());

    // Status characteristic (Read + Notify)
    _status_char = service->createCharacteristic(
        BLE_STATUS_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    _status_char->addDescriptor(new BLE2902());

    service->start();

    BLEAdvertising* adv = BLEDevice::getAdvertising();
    adv->addServiceUUID(BLE_SERVICE_UUID);
    adv->setScanResponse(true);
    adv->setMinPreferred(0x06);   // Helps with iPhone connection stability
    adv->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    Serial.println("[BLE] Advertising as '" BLE_DEVICE_NAME "'");
}

BleCommand ble_get_command() {
    BleCommand cmd = _pending;
    _pending = BLE_NONE;
    return cmd;
}

void ble_notify_status(bool gate_open, int spaces) {
    if (!_client_connected || !_status_char) return;
    // Format: "OPEN,8" or "CLOSED,5"
    String payload = String(gate_open ? "OPEN" : "CLOSED") + "," + String(spaces);
    _status_char->setValue(payload.c_str());
    _status_char->notify();
}
