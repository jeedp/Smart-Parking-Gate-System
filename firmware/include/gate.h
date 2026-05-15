#pragma once
#include <Arduino.h>
#include "pins.h"
#include "config.h"

// Which gate to operate
enum GateId { GATE_ENTRY, GATE_EXIT };

void gate_init();
void gate_open(GateId gate);
void gate_close(GateId gate);
bool gate_is_open(GateId gate);
