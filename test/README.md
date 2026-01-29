# Test Directory

Unit tests for the TAF Analyzer project using PlatformIO's testing framework with Unity.

## 🎯 Purpose

Contains unit tests to validate individual components and integration points of the layered architecture.

## 🧪 Test Framework

- **PlatformIO Test Runner** - Built-in test framework
- **Unity** - C unit testing framework
- **ESP32 Target** - Tests run on actual hardware

## 📁 Structure

```
test/
├── README.md                    # This file
└── test_network_layer.cpp       # Network Layer tests
```

## 🚀 Running Tests

### Run All Tests
```bash
pio test
```

### Run Specific Test
```bash
pio test -f test_network_layer
```

### Verbose Output
```bash
pio test -v
```

## 📝 Writing Tests

### Test File Template

```cpp
#include <Arduino.h>
#include <unity.h>
#include "layers/network/NetworkLayer.h"
#include "layers/data/DataLayer.h"

void test_feature_name(void) {
    // Arrange
    NetworkLayer network;
    TEST_ASSERT_TRUE(network.init());
    
    // Act
    bool result = network.hasSubscribers("test/topic");
    
    // Assert
    TEST_ASSERT_FALSE(result);
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_feature_name);
    UNITY_END();
}

void loop() {
    // Tests run once in setup()
}
```

### Unity Assertions

```cpp
// Boolean assertions
TEST_ASSERT_TRUE(condition);
TEST_ASSERT_FALSE(condition);

// Equality assertions
TEST_ASSERT_EQUAL(expected, actual);
TEST_ASSERT_EQUAL_INT(expected, actual);
TEST_ASSERT_EQUAL_STRING(expected, actual);

// Null checks
TEST_ASSERT_NULL(pointer);
TEST_ASSERT_NOT_NULL(pointer);

// Custom messages
TEST_PASS_MESSAGE("Test passed with custom message");
TEST_FAIL_MESSAGE("Test failed with custom message");
```

## 🧩 Test Coverage

### Network Layer Tests
- ✅ Semaphore protection during subscribe
- ✅ Multiple subscribers per topic
- ✅ Publish to subscribers
- ✅ Thread safety under concurrent access

### Data Layer Tests (TODO)
- [ ] Set and get operations
- [ ] TTL expiration
- [ ] Key existence checks
- [ ] Thread safety

### Application Tests (TODO)
- [ ] Bluetooth connection handling
- [ ] LED command processing
- [ ] MPU sensor data collection
- [ ] Inter-application communication

## ⚠️ Important Notes

### Hardware Requirements
- Tests run on **actual ESP32 hardware**
- Serial connection required for output
- Upload port must be accessible

### Test Isolation
- Each test should be independent
- Use `setup()` to initialize
- Clean up resources after tests

### Async Testing
For async operations (RTOS tasks):
```cpp
void test_async_operation(void) {
    // Start async operation
    startAsyncTask();
    
    // Wait for completion
    delay(100);
    
    // Assert result
    TEST_ASSERT_TRUE(taskCompleted());
}
```

## 🐛 Debugging Tests

### Serial Output
```cpp
Serial.printf("Debug: value = %d\n", value);
TEST_ASSERT_TRUE(value > 0);
```

### Test Isolation
If tests interfere with each other:
- Add cleanup in `setUp()` and `tearDown()`
- Reset static variables
- Clear network subscriptions

### Failed Assertions
Unity provides clear failure messages:
```
test/test_network_layer.cpp:42:test_subscribe:FAIL: Expected TRUE Was FALSE
```

## 📊 Test Results

Test runner provides summary:
```
Environment    Test           Status    Duration
-------------  -------------  --------  ----------
esp32cam       test_semaphore PASSED    00:00:02.1
esp32cam       test_publish   PASSED    00:00:01.8
==================================================
2 test cases: 2 succeeded in 00:00:03.9
```

## 🔮 Future Tests

- [ ] End-to-end integration tests
- [ ] Performance benchmarks
- [ ] Memory leak detection
- [ ] Stress tests (high message volume)
- [ ] Failure recovery tests
- [ ] Mock hardware tests (on native platform)

## 🔗 Documentation

- [PlatformIO Testing](https://docs.platformio.org/en/latest/advanced/unit-testing/index.html)
- [Unity Framework](https://github.com/ThrowTheSwitch/Unity)
- [ESP32 Testing Best Practices](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/unit-tests.html)

---

**Part of the TAF Analyzer test suite**
