#include <Arduino.h>
#include <unity.h>
#include "gate.h"

void test_both_gates_start_closed() {
    gate_init();
    TEST_ASSERT_FALSE(gate_is_open(GATE_ENTRY));
    TEST_ASSERT_FALSE(gate_is_open(GATE_EXIT));
}

void test_entry_gate_opens_and_closes() {
    gate_open(GATE_ENTRY);
    TEST_ASSERT_TRUE(gate_is_open(GATE_ENTRY));
    TEST_ASSERT_FALSE(gate_is_open(GATE_EXIT));   // Exit unaffected

    gate_close(GATE_ENTRY);
    TEST_ASSERT_FALSE(gate_is_open(GATE_ENTRY));
}

void test_exit_gate_opens_and_closes() {
    gate_open(GATE_EXIT);
    TEST_ASSERT_TRUE(gate_is_open(GATE_EXIT));
    TEST_ASSERT_FALSE(gate_is_open(GATE_ENTRY));  // Entry unaffected

    gate_close(GATE_EXIT);
    TEST_ASSERT_FALSE(gate_is_open(GATE_EXIT));
}

void test_open_entry_is_idempotent() {
    gate_open(GATE_ENTRY);
    gate_open(GATE_ENTRY);   // Second call should not move servo again
    TEST_ASSERT_TRUE(gate_is_open(GATE_ENTRY));
    gate_close(GATE_ENTRY);
}

void test_close_exit_is_idempotent() {
    gate_close(GATE_EXIT);   // Already closed — should not move servo
    TEST_ASSERT_FALSE(gate_is_open(GATE_EXIT));
}

void test_both_gates_can_be_open_simultaneously() {
    gate_open(GATE_ENTRY);
    gate_open(GATE_EXIT);
    TEST_ASSERT_TRUE(gate_is_open(GATE_ENTRY));
    TEST_ASSERT_TRUE(gate_is_open(GATE_EXIT));
    gate_close(GATE_ENTRY);
    gate_close(GATE_EXIT);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_both_gates_start_closed);
    RUN_TEST(test_entry_gate_opens_and_closes);
    RUN_TEST(test_exit_gate_opens_and_closes);
    RUN_TEST(test_open_entry_is_idempotent);
    RUN_TEST(test_close_exit_is_idempotent);
    RUN_TEST(test_both_gates_can_be_open_simultaneously);
    UNITY_END();
}

void loop() {}
