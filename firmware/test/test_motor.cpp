#include <Arduino.h>
#include <unity.h>
#include "motor.h"

void test_initial_state_is_closed() {
    motor_init();
    TEST_ASSERT_FALSE(motor_is_open());
}

void test_open_sets_state() {
    motor_open_gate();
    TEST_ASSERT_TRUE(motor_is_open());
}

void test_close_sets_state() {
    motor_close_gate();
    TEST_ASSERT_FALSE(motor_is_open());
}

void test_open_is_idempotent() {
    motor_open_gate();
    motor_open_gate();   // Should not double-run the motor
    TEST_ASSERT_TRUE(motor_is_open());
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_initial_state_is_closed);
    RUN_TEST(test_open_sets_state);
    RUN_TEST(test_close_sets_state);
    RUN_TEST(test_open_is_idempotent);
    UNITY_END();
}

void loop() {}
