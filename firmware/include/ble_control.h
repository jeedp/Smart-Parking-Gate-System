#pragma once
#include <Arduino.h>

// Commands received from BLE client
enum BleCommand { BLE_NONE, BLE_OPEN, BLE_CLOSE, BLE_DISCONNECT };

void       ble_init();
BleCommand ble_get_command();             // Returns and clears pending command
void       ble_notify_status(bool gate_open, int spaces);  // Push status to connected client
void       ble_disconnect_client();       // Gracefully disconnect current client
