# BluetoothLed Coordinator

A coordinator application demonstrating topic-based orchestration by linking Bluetooth connection state to LED indicators.

## 🎯 Purpose

Demonstrates the **coordinator pattern** - an application with no hardware logic that orchestrates behavior between other applications using only topic-based messaging.

## 🧠 Coordinator Pattern

This application:
- ❌ Has **no hardware** dependencies
- ❌ Has **no direct references** to other applications
- ✅ Only communicates via **topic subscriptions and publications**
- ✅ Provides **loose coupling** between Bluetooth and LED

## 📡 Topics

### Subscribes To
- `bluetooth/connected` - Bluetooth connection established
- `bluetooth/disconnected` - Bluetooth connection lost

### Publishes
- `led/2/command` - LED control commands

## 🔄 Operation Flow

```
┌─────────────────────────────────────────────┐
│ Bluetooth device connects                  │
│ → Bluetooth app publishes                  │
│   bluetooth/connected                      │
└─────────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────────┐
│ BluetoothLed coordinator receives event    │
│ → Decides LED should blink                 │
└─────────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────────┐
│ Publishes led/2/command = "blink"         │
└─────────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────────┐
│ LED app receives command                   │
│ → Starts blinking                          │
└─────────────────────────────────────────────┘


┌─────────────────────────────────────────────┐
│ Bluetooth device disconnects               │
│ → Bluetooth app publishes                  │
│   bluetooth/disconnected                   │
└─────────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────────┐
│ BluetoothLed coordinator receives event    │
│ → Decides LED should turn off              │
└─────────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────────┐
│ Publishes led/2/command = "off"            │
└─────────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────────┐
│ LED app receives command                   │
│ → Turns off                                │
└─────────────────────────────────────────────┘
```

## 🚀 Usage Example

```cpp
// In main.cpp
bluetoothLedApp = new BluetoothLed();
bluetoothLedApp->setNetworkLayer(networkLayer)->setDataLayer(dataLayer);
bluetoothLedApp->setup();
bluetoothLedApp->createTask("BluetoothLEDApp", 4096, 3, tskNO_AFFINITY, 50);
```

That's it! The coordinator automatically links Bluetooth state to LED behavior.

## ⚙️ Configuration

- **Update Frequency**: 50ms (20Hz)
- **Stack Size**: 4096 bytes
- **Priority**: 3 (higher than apps it coordinates)

## 💡 Why Coordinators?

### Without Coordinator (Tight Coupling)
```cpp
// ❌ Bad: Direct dependency
class Bluetooth {
    LED* led_;  // Direct reference
    
    void onConnect() {
        led_->blink();  // Tight coupling
    }
};
```

### With Coordinator (Loose Coupling)
```cpp
// ✅ Good: Topic-based coordination
class Bluetooth {
    void onConnect() {
        networkLayer->publish("bluetooth/connected", ...);
        // Bluetooth doesn't know about LED
    }
};

class BluetoothLed {
    void onBluetoothConnected(...) {
        networkLayer->publish("led/2/command", "blink", ...);
        // Coordinator bridges the gap
    }
};
```

## 🎯 Benefits

1. **Modularity** - Bluetooth and LED apps remain independent
2. **Testability** - Each app can be tested in isolation
3. **Flexibility** - Change coordination logic without modifying apps
4. **Reusability** - Apps can be used in different contexts
5. **Scalability** - Add more coordinators for complex behaviors

## 🧩 Creating Custom Coordinators

Template for new coordinators:

```cpp
class MyCoordinator : public ApplicationInterface {
public:
    bool setup() override {
        // Subscribe to input topics
        networkLayer->subscribe("input/topic", "MyCoordinator", 
            [this](const uint8_t* data, size_t len, const std::string& topic) {
                this->onInputEvent(data, len, topic);
            });
        return true;
    }
    
    void update() override {
        // Optional periodic logic
    }
    
private:
    void onInputEvent(const uint8_t* data, size_t len, const std::string& topic) {
        // Process event
        // Publish commands to output topics
        networkLayer->publish("output/topic", responseData, len, "MyCoordinator");
    }
};
```

## 🎨 Example Coordinator Ideas

### CameraLed Coordinator
```
camera/capturing → led/3/command (blink while capturing)
camera/captured  → led/3/command (flash once on capture)
```

### SensorAlert Coordinator
```
mpu/accel → analyze → led/4/command (alert on high G-force)
mpu/temp  → check  → bluetooth/transmit (send warning)
```

### BatteryMonitor Coordinator
```
battery/low → led/5/command (slow blink)
battery/critical → led/5/command (fast blink) + bluetooth/transmit
```

## 📝 Design Philosophy

Coordinators should:
- ✅ Only contain **orchestration logic**
- ✅ Have **no hardware** access
- ✅ Use **only topic-based** communication
- ✅ Be **easily replaceable** without affecting apps
- ✅ Implement **single responsibility** (one coordination task)

## 🔗 Related Applications

- **Bluetooth** - Source of connection events
- **LED** - Target of LED commands
- Both applications work independently and don't know about this coordinator

---

**Part of the TAF Analyzer Application Layer - Demonstrating the Coordinator Pattern**
