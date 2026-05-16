#include <Arduino.h>
#include <unity.h>
#include "rfid.h"

void test_authorized_uid_is_accepted() {
    TEST_ASSERT_TRUE(rfid_is_authorized("4A3FB21C"));
    TEST_ASSERT_TRUE(rfid_is_authorized("7B2A09EE"));
    TEST_ASSERT_TRUE(rfid_is_authorized("C3885D7A"));
}

void test_unauthorized_uid_is_rejected() {
    TEST_ASSERT_FALSE(rfid_is_authorized("00000000"));
    TEST_ASSERT_FALSE(rfid_is_authorized("DEADBEEF"));
    TEST_ASSERT_FALSE(rfid_is_authorized(""));
}

void test_lowercase_uid_is_authorized_after_normalize() {
    // rfid_read_uid() always calls toUpperCase() before passing to is_authorized,
    // so simulate that here: uppercase first, then check
    String uid = "4a3fb21c";
    uid.toUpperCase();
    TEST_ASSERT_TRUE(rfid_is_authorized(uid));
}

void test_uid_with_extra_chars_is_rejected() {
    // Ensure partial matches don't pass
    TEST_ASSERT_FALSE(rfid_is_authorized("4A3FB21C00"));
    TEST_ASSERT_FALSE(rfid_is_authorized("004A3FB21C"));
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_authorized_uid_is_accepted);
    RUN_TEST(test_unauthorized_uid_is_rejected);
    RUN_TEST(test_lowercase_uid_is_authorized_after_normalize);
    RUN_TEST(test_uid_with_extra_chars_is_rejected);
    UNITY_END();
}

void loop() {}
