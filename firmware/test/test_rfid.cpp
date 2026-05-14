#include <Arduino.h>
#include <unity.h>
#include "rfid.h"

void test_authorized_uid_is_accepted() {
    // First UID in the authorized list should return true
    TEST_ASSERT_TRUE(rfid_is_authorized("4A3FB21C"));
}

void test_unauthorized_uid_is_rejected() {
    TEST_ASSERT_FALSE(rfid_is_authorized("00000000"));
    TEST_ASSERT_FALSE(rfid_is_authorized("DEADBEEF"));
}

void test_uid_case_insensitive() {
    // UIDs should match regardless of case after toUpperCase()
    TEST_ASSERT_TRUE(rfid_is_authorized("4a3fb21c"));
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_authorized_uid_is_accepted);
    RUN_TEST(test_unauthorized_uid_is_rejected);
    RUN_TEST(test_uid_case_insensitive);
    UNITY_END();
}

void loop() {}
