# TAF Analyzer

ESP32-CAM firmware implementing a layered architecture with FreeRTOS-based concurrent applications and topic-based inter-process communication.

## Overview

TAF Analyzer is an embedded systems framework demonstrating production-grade design patterns for ESP32 development:

- **FreeRTOS Architecture** - Concurrent task execution with configurable priorities and frequencies
- **Layered Design** - Separation between infrastructure (network, data) and application logic
- **Topic-Based IPC** - Publish-subscribe messaging for decoupled inter-application communication
- **Modular Applications** - Self-contained components with independent lifecycles
- **Thread-Safe Operations** - Semaphore and mutex protection for shared resources

## System Architecture

### Layered Structure

```
┌─────────────────────────────────────────────────────┐
│                 Application Layer                   │
│  Concurrent RTOS Tasks with Independent Lifecycles  │
│  - Bluetooth Serial (20Hz)                          │
│  - LED Control (10Hz)                               │
│  - MPU6050 Sensor (100Hz)                           │
│  - Application Coordinators                         │
└─────────────────────────────────────────────────────┘
                         ↕
┌─────────────────────────────────────────────────────┐
│              Network Layer (Message Broker)         │
│  Thread-Safe Topic-Based Publish-Subscribe          │
│  - FreeRTOS Semaphore Protection                    │
│  - Asynchronous Message Delivery                    │
└─────────────────────────────────────────────────────┘
                         ↕
┌─────────────────────────────────────────────────────┐
│          Data Layer (Key-Value Store)               │
│  Thread-Safe Storage with TTL Support               │
│  - FreeRTOS Mutex Protection                        │
│  - Background Cleanup Task                          │
└─────────────────────────────────────────────────────┘
```

### RTOS Task Architecture

Each application runs as an independent FreeRTOS task with dedicated stack, priority, and update frequency:

| Application      | Task Name       | Stack | Priority | Frequency | Core    |
|------------------|-----------------|-------|----------|-----------|---------|
| Bluetooth Serial | BluetoothApp    | 4KB   | 2        | 50ms      | Any     |
| LED Controller   | LEDApp          | 4KB   | 2        | 100ms     | Any     |
| MPU6050 Sensor   | MPUApp          | 4KB   | 2        | 10ms      | Any     |
| BT-LED Coord.    | BluetoothLEDApp | 4KB   | 3        | 50ms      | Any     |

**Task Scheduling**: Preemptive multitasking managed by FreeRTOS scheduler with tick rate of 1ms.

**Synchronization**: Applications communicate exclusively through the Network Layer's topic system, eliminating direct dependencies and race conditions.

## Hardware Requirements

- **Board**: ESP32-CAM (AI Thinker)
- **MCU**: ESP32 dual-core @ 240MHz, 320KB RAM, 4MB Flash
- **Peripherals**:
  - Built-in Bluetooth Classic (SPP profile)
  - Camera module (OV2640)
  - MPU6050 IMU on I2C (SDA=GPIO14, SCL=GPIO15)
  - LED on GPIO2

## Quick Start

### 1. Build Firmware

```bash
cd TAFAnalizer
pio run
```

### 2. Upload to ESP32-CAM

```bash
pio run --target upload --upload-port /dev/ttyUSB0
```

### 3. Connect via Bluetooth

**Requirements**: Android/iOS device with Serial Bluetooth Terminal app

**Recommended Apps**:
- Android: "Serial Bluetooth Terminal" by Kai Morich
- iOS: "Bluetooth Terminal" or similar SPP terminal app

**Connection Steps**:
1. Open Bluetooth settings on your phone
2. Scan for devices and pair with `ESP32-CAM-TAF`
3. Open Serial Bluetooth Terminal app
4. Connect to `ESP32-CAM-TAF`
5. Device is ready for communication

### 4. Monitor System

```bash
pio device monitor --port /dev/ttyUSB0 --baud 115200
```

**Expected Output**:
```
[NetworkLayer] Initialized with direct callback delivery
[DataLayer] Initialized with TTL support
[Bluetooth] Bluetooth started. Pair with ESP32-CAM-TAF
[LED] Setup complete for GPIO 2
[BluetoothLed] Setup complete
RTOS tasks initialized - applications now running concurrently
```

## Topic-Based Communication

Applications communicate through named topics without direct coupling:

```cpp
// Bluetooth publishes connection events
bluetooth/connected      → Published when device connects
bluetooth/disconnected   → Published when device disconnects

// LED subscribes to commands
led/2/command           → Receives: "on", "off", "toggle", "blink"
led/2/state             → Publishes current state (0/1)

// Coordinator links them
BluetoothLed: listens to bluetooth/* → publishes to led/*/command
```

This architecture enables:
- **Modularity**: Add/remove applications without affecting others
- **Testability**: Applications can be tested in isolation
- **Scalability**: New functionality through new topics and coordinators

## Project Structure

```
TAFAnalizer/
├── src/
│   ├── main.cpp                           # System initialization
│   └── layers/
│       ├── network/                       # Topic-based message broker
│       │   ├── NetworkLayer.h/cpp
│       │   └── README.md
│       ├── data/                          # Key-value store with TTL
│       │   ├── DataLayer.h/cpp
│       │   └── README.md
│       └── application/                   # Modular applications
│           ├── ApplicationInterface.h/cpp # Base class for all apps
│           ├── bluetooth/                 # Bluetooth Serial communication
│           ├── led/                       # LED control
│           ├── mpu/                       # MPU6050 IMU sensor
│           └── bluetooth_led/             # Coordinator example
├── test/                                  # Unit tests
├── platformio.ini                         # Build configuration
└── README.md                              # This file
```

Each subdirectory contains detailed README documentation for its specific domain.

## System Capabilities

### Implemented

- **Bluetooth Serial Communication**: Device-to-phone data exchange via SPP
- **LED Control**: GPIO-based visual indicators with command interface
- **Sensor Integration**: MPU6050 accelerometer and gyroscope data collection
- **Application Coordination**: Event-driven behavior orchestration between applications
- **Thread-Safe Messaging**: Inter-task communication without race conditions
- **Dynamic Storage**: Key-value store with automatic expiration

### Framework Ready

- Camera image capture and streaming
- WiFi connectivity layer
- Web server for remote configuration
- OTA firmware updates
- SD card logging

## Design Principles

### Modularity

Applications are self-contained units with no direct references to each other. New applications integrate by:
1. Inheriting from `ApplicationInterface`
2. Subscribing to relevant topics
3. Publishing events and data to topics
4. Running as independent RTOS tasks

### Scalability

The system scales horizontally through:
- Adding new application tasks (limited by available RAM)
- Creating coordinator applications for complex behaviors
- Defining new topics for new data flows
- No modification to existing applications required

### Thread Safety

All shared resources protected by FreeRTOS synchronization primitives:
- **Semaphores** for subscriber registry access
- **Mutexes** for data store access
- **Message queues** implicit in topic delivery
- No direct memory sharing between tasks

### Performance

Resource usage (with 4 concurrent applications):
- **RAM**: 40KB / 320KB (12.3%)
- **Flash**: 1.15MB / 3MB (36.6%)
- **CPU**: Minimal idle time, responsive task switching

## Documentation

- [`src/layers/network/README.md`](src/layers/network/README.md) - Network Layer API and patterns
- [`src/layers/data/README.md`](src/layers/data/README.md) - Data Layer storage interface
- [`src/layers/application/README.md`](src/layers/application/README.md) - Application development guide
- Individual application READMEs in respective subdirectories

## Dependencies

Managed automatically by PlatformIO:

- **Framework**: Arduino-ESP32 (3.20017.241212)
- **Platform**: Espressif32 ^6.0.0
- **Libraries**:
  - Adafruit MPU6050 (I2C sensor driver)
  - Adafruit Unified Sensor (sensor abstraction)
  - BluetoothSerial (ESP32 SPP profile)

## Development

### Adding New Applications

See [`src/layers/application/README.md`](src/layers/application/README.md) for complete guide.

Minimal example:
```cpp
class MyApp : public ApplicationInterface {
public:
    bool setup() override {
        networkLayer_->subscribe("input/topic", "MyApp", callback);
        return true;
    }
    
    void update() override {
        // Runs at configured frequency
    }
};
```

### Testing

```bash
pio test                     # Run all tests
pio test -f test_name        # Run specific test
```

See [`test/README.md`](test/README.md) for testing guidelines.

## Technical Specifications

- **MCU**: ESP32 dual-core Xtensa LX6
- **Clock**: 240 MHz
- **RAM**: 520 KB SRAM (320KB available to applications)
- **Flash**: 4 MB
- **RTOS**: FreeRTOS (integrated with ESP-IDF)
- **Scheduler**: Preemptive, priority-based
- **Tick Rate**: 1 ms (configurable)
- **Max Tasks**: Limited by available stack memory
- **Build System**: PlatformIO 6.x
- **Toolchain**: Xtensa GCC 8.4.0

---

**License**: MIT  
**Target Platform**: ESP32-CAM (AI Thinker)  
**Framework**: Arduino + FreeRTOS

## 🔮 Future Enhancements

- [ ] WiFi connectivity layer
- [ ] Web server for configuration
- [ ] OTA (Over-The-Air) updates
- [ ] SD card logging
- [ ] Advanced camera features
- [ ] MQTT integration
- [ ] Configuration persistence

---

**Built with ❤️ for ESP32-CAM**
