#include "ble_control.h"
#include "config.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

static BLECharacteristic* _gate_char   = nullptr;
static BLECharacteristic* _status_char = nullptr;
static volatile BleCommand _pending    = BLE_NONE;
static bool _client_connected          = false;

// ─── Server callbacks ─────────────────────────────────────────────────────────
class ServerCB : public BLEServerCallbacks {
    void onConnect(BLEServer*)    override { _client_connected = true;  Serial.println("[BLE] Client connected"); }
    void onDisconnect(BLEServer* s) override {
        _client_connected = false;
        Serial.println("[BLE] Client disconnected — restarting advertising");
        s->getAdvertising()->start();
    }
};

// ─── Gate characteristic write callback ───────────────────────────────────────
class GateCB : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* c) override {
        String val = c->getValue().c_str();
        val.toUpperCase();
        if      (val == "OPEN")    _pending = BLE_OPEN;
        else if (val == "CLOSE")   _pending = BLE_CLOSE;
        else if (val == "REFRESH") _pending = BLE_REFRESH;
        Serial.printf("[BLE] Received command: %s\n", val.c_str());
    }
};

void ble_init() {
    BLEDevice::init(BLE_DEVICE_NAME);
    BLEServer* server = BLEDevice::createServer();
    server->setCallbacks(new ServerCB());

    BLEService* service = server->createService(BLE_SERVICE_UUID);

    // Gate command characteristic (Write)
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
    adv->start();

    Serial.println("[BLE] Advertising as '" BLE_DEVICE_NAME "'");
}

BleCommand ble_get_command() {
    BleCommand cmd = _pending;
    _pending = BLE_NONE;
    return cmd;
}

void ble_notify_status(bool gate_open, int spaces) {
    if (!_client_connected || !_status_char) return;
    String payload = String(gate_open ? "OPEN" : "CLOSED") + "," + String(spaces);
    _status_char->setValue(payload.c_str());
    _status_char->notify();
}
