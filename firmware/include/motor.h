#pragma once
#include <Arduino.h>
#include "pins.h"
#include "config.h"

void motor_init();
void motor_open_gate();
void motor_close_gate();
void motor_stop();
bool motor_is_open();
