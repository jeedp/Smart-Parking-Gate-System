#pragma once
#include <Arduino.h>

// Commands received from BLE client
enum BleCommand { BLE_NONE, BLE_OPEN, BLE_CLOSE, BLE_REFRESH };

void       ble_init();
BleCommand ble_get_command();   // Returns and clears pending command
void       ble_notify_status(bool gate_open, int spaces);
