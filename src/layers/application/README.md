# Application Layer

A modular application framework built on the `ApplicationInterface` base class, enabling concurrent execution of independent applications with shared infrastructure.

## 🎯 Purpose

The Application Layer provides a standardized interface for creating modular, self-contained applications that run as concurrent RTOS tasks. Each application encapsulates its hardware logic and communicates via the Network Layer's topic-based messaging system.

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────┐
│          ApplicationInterface (Base)                │
│  - Task Management                                  │
│  - Lifecycle Methods (setup/update)                 │
│  - Layer Dependencies (Network, Data)               │
└─────────────────────────────────────────────────────┘
                         ↑
         ┌───────────────┼───────────────┬────────────┐
         │               │               │            │
    ┌────────┐      ┌────────┐     ┌────────┐   ┌─────────┐
    │ Bluetooth│      │   LED  │     │  MPU   │   │  Camera │
    │          │      │        │     │        │   │         │
    └────────┘      └────────┘     └────────┘   └─────────┘
```

## 🔑 ApplicationInterface Base Class

All applications inherit from `ApplicationInterface`, which provides:

### Core Features

1. **Dependency Injection** - Network and Data layer references
2. **RTOS Task Management** - Automatic task creation and cleanup
3. **Custom Update Frequencies** - Per-application task rates
4. **Lifecycle Hooks** - setup() and update() virtual methods

### API Reference

```cpp
class ApplicationInterface {
public:
    // Lifecycle methods (must override)
    virtual bool setup() = 0;     // Initialize application
    virtual void update() = 0;    // Periodic update loop
    
    // Dependency injection (fluent interface)
    ApplicationInterface* setNetworkLayer(NetworkLayer* network);
    ApplicationInterface* setDataLayer(DataLayer* data);
    
    // Task management
    bool createTask(
        const char* name,           // Task name
        uint32_t stackSize,         // Stack size in bytes
        UBaseType_t priority,       // FreeRTOS priority
        BaseType_t coreID,          // CPU core (0, 1, or tskNO_AFFINITY)
        uint32_t frequencyMs        // Update frequency in milliseconds
    );
    
    void stopTask();                // Stop and cleanup task
    
protected:
    NetworkLayer* networkLayer_;    // Access to messaging
    DataLayer* dataLayer_;          // Access to storage
};
```

## 📋 Creating New Applications

### Step-by-Step Guide

1. **Create Header File** (`MyApp.h`)

```cpp
#ifndef MYAPP_H
#define MYAPP_H

#include "../ApplicationInterface.h"

class MyApp : public ApplicationInterface {
public:
    MyApp();
    ~MyApp();
    
    bool setup() override;
    void update() override;
    
private:
    bool initialized_;
    // Application-specific members
    void onTopicMessage(const uint8_t* data, size_t len, const std::string& topic);
};

#endif
```

2. **Create Implementation File** (`MyApp.cpp`)

```cpp
#include "MyApp.h"
#include <Arduino.h>

MyApp::MyApp() : initialized_(false) {
    Serial.println("[MyApp] Created");
}

MyApp::~MyApp() {
    if (initialized_) {
        networkLayer_->unsubscribe("my/topic", "MyApp");
        Serial.println("[MyApp] Cleaned up");
    }
}

bool MyApp::setup() {
    if (!networkLayer_ || !dataLayer_) {
        Serial.println("[MyApp] ERROR: Missing layer dependencies");
        return false;
    }
    
    // Initialize hardware
    pinMode(LED_PIN, OUTPUT);
    
    // Subscribe to topics
    auto callback = [this](const uint8_t* data, size_t len, const std::string& topic) {
        this->onTopicMessage(data, len, topic);
    };
    
    if (!networkLayer_->subscribe("my/topic", "MyApp", callback)) {
        Serial.println("[MyApp] Failed to subscribe");
        return false;
    }
    
    initialized_ = true;
    Serial.println("[MyApp] Setup complete");
    return true;
}

void MyApp::update() {
    if (!initialized_) {
        return;
    }
    
    // Periodic update logic
    // Runs at the frequency specified in createTask()
}

void MyApp::onTopicMessage(const uint8_t* data, size_t len, const std::string& topic) {
    Serial.printf("[MyApp] Received %d bytes on %s\n", len, topic.c_str());
    // Handle message
}
```

3. **Register in main.cpp**

```cpp
#include "layers/application/myapp/MyApp.h"

ApplicationInterface* myApp = nullptr;

void setup() {
    // ... initialize layers ...
    
    myApp = new MyApp();
    myApp->setNetworkLayer(networkLayer)->setDataLayer(dataLayer);
    
    if (!myApp->setup()) {
        Serial.println("Failed to setup MyApp");
        delete myApp;
        myApp = nullptr;
    }
    
    // Create RTOS task with 100ms update frequency (10Hz)
    if (myApp) {
        if (!myApp->createTask("MyApp", 4096, 2, tskNO_AFFINITY, 100)) {
            Serial.println("Failed to create MyApp task");
        }
    }
}
```

## 🎯 Existing Applications

### Bluetooth Application

**Purpose**: Bluetooth Serial communication layer

**Topics**:
- Subscribes: `bluetooth/transmit` (relay data over Bluetooth)
- Publishes: `bluetooth/connected`, `bluetooth/disconnected`

**Features**:
- Connection status monitoring
- Data transmission over Bluetooth Serial
- Automatic connection state events

**Update Frequency**: 50ms (20Hz)

**Files**: `bluetooth/Bluetooth.h`, `bluetooth/Bluetooth.cpp`

---

### LED Application

**Purpose**: GPIO-based LED control

**Topics**:
- Subscribes: `led/{pin}/command` (control commands)
- Publishes: `led/{pin}/state`, `led/{pin}/mode` (status broadcasts)

**Features**:
- Pin-based topic namespaces
- Periodic status broadcasting
- Multiple LED support (instantiate per pin)
- Commands: on, off, toggle, blink

**Update Frequency**: 100ms (10Hz)

**Files**: `led/LED.h`, `led/LED.cpp`

---

### MPU6050 Application

**Purpose**: IMU sensor data collection

**Topics**:
- Publishes: `mpu/accel`, `mpu/gyro`, `mpu/temp`

**Features**:
- Self-contained Adafruit_MPU6050 integration
- High-frequency sensor reading
- Binary data publishing (float arrays)
- I2C communication (SDA=14, SCL=15)

**Update Frequency**: 10ms (100Hz)

**Files**: `mpu/MPU.h`, `mpu/MPU.cpp`

---

### Camera Application (Framework)

**Purpose**: ESP32-CAM image capture

**Topics**:
- Subscribes: `capture/start`, `capture/stop`
- Publishes: `capture/status`, `image/data`

**Features**:
- Camera initialization and configuration
- Image capture on command
- Image data streaming

**Update Frequency**: Variable (event-driven)

**Files**: `camera/Camera.h`, `camera/Camera.cpp`

---

### BluetoothLed Coordinator

**Purpose**: Links Bluetooth connection state to LED indicators

**Topics**:
- Subscribes: `bluetooth/connected`, `bluetooth/disconnected`
- Publishes: `led/2/command`

**Features**:
- Application coordination example
- No hardware logic (pure coordinator)
- Event-driven LED control
- Demonstrates topic-based orchestration

**Update Frequency**: 50ms (20Hz)

**Files**: `bluetooth_led/BluetoothLed.h`, `bluetooth_led/BluetoothLed.cpp`

## 🧪 Design Patterns

### Self-Contained Hardware Logic

Applications encapsulate their own hardware:

```cpp
class LED : public ApplicationInterface {
private:
    int pin_;              // Hardware pin
    bool state_;           // Current state
    
    void turnOn() { digitalWrite(pin_, HIGH); }
    void turnOff() { digitalWrite(pin_, LOW); }
};
```

### Topic-Based Communication

No direct application references:

```cpp
// ❌ Bad: Direct coupling
bluetoothApp->sendData(data);

// ✅ Good: Topic-based
networkLayer->publish("bluetooth/transmit", data, len, "MyApp");
```

### Coordinator Pattern

Coordinators link applications without direct coupling:

```cpp
class BluetoothLed : public ApplicationInterface {
    // Subscribes to Bluetooth events
    // Publishes LED commands
    // No direct references to either application
};
```

### Static Timing Variables

Persistent state within methods:

```cpp
void update() {
    static unsigned long lastBroadcast = 0;
    if (millis() - lastBroadcast > 1000) {
        broadcastStatus();
        lastBroadcast = millis();
    }
}
```

## ⚠️ Best Practices

### Setup Method
- ✅ Check for layer dependencies
- ✅ Initialize hardware
- ✅ Subscribe to all needed topics
- ✅ Return `false` on failure

### Update Method
- ✅ Check `initialized_` flag
- ✅ Keep execution time short (< frequency period)
- ✅ Use static variables for timing
- ✅ Avoid blocking operations

### Thread Safety
- ✅ Don't call blocking functions in callbacks
- ✅ Avoid Serial prints in high-frequency tasks
- ✅ Use proper task priorities
- ✅ Be aware of shared resource access

### Memory Management
- ✅ Monitor stack usage (adjust stackSize if needed)
- ✅ Clean up in destructor (unsubscribe, free resources)
- ✅ Avoid dynamic allocation in update()
- ✅ Use std::vector for variable-size data

## 📊 Task Configuration Guidelines

| Application Type | Stack Size | Priority | Frequency | Notes |
|------------------|------------|----------|-----------|-------|
| Sensors (fast)   | 4096       | 2-3      | 10-100ms  | Real-time data |
| Communication    | 4096-8192  | 2        | 50-100ms  | I/O operations |
| Coordinators     | 2048-4096  | 3        | 50-200ms  | Event-driven |
| Display/UI       | 4096       | 1-2      | 100-500ms | User interaction |
| Background       | 2048       | 1        | 500-1000ms| Housekeeping |

## 🐛 Debugging Applications

### Serial Logging
```cpp
Serial.printf("[AppName] Message with %d value\n", value);
```

### Task Monitoring
```cpp
UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(nullptr);
Serial.printf("Stack remaining: %d bytes\n", stackHighWaterMark);
```

### Common Issues

**Task Not Running:**
- Check `createTask()` return value
- Verify `setup()` returns `true`
- Check stack size is sufficient

**Memory Issues:**
- Increase stack size in `createTask()`
- Reduce data buffer sizes
- Check for memory leaks

**Timing Problems:**
- Adjust update frequency
- Verify task priority
- Check for blocking in `update()`

## 🔗 Related Documentation

- `../../network/README.md` - Network Layer messaging
- `../../data/README.md` - Data Layer storage
- Individual application README files in subdirectories

---

**Part of the TAF Analyzer layered architecture**
