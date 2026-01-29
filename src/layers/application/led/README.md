# LED Application# LED Application



GPIO-based LED control with pin-specific topic namespaces and periodic status broadcasting.The LED application provides visual status indication and feedback for the TAF (Terrain Analysis Framework) system state.



## 🎯 Purpose## Features



Provides simple LED control via topic-based commands. Supports multiple LED instances (one per GPIO pin) with independent control and status reporting.- **Status indication**: Visual feedback for system state (off/on/blinking)

- **Event-driven**: Responds to system events (capture start/stop, Bluetooth connection)

## 🔌 Hardware- **Network integration**: Receives status commands and publishes status changes

- **Configurable blinking**: Adjustable blink intervals for different states

- **GPIO Pin**: Configurable (default: GPIO 2)- **State persistence**: Current status stored in Data Layer

- **LED**: Standard LED or built-in LED

- **No external hardware required** for built-in LED## Dependencies



## 📡 Topics- NetworkLayer: For receiving commands and publishing status changes

- DataLayer: For storing current LED status

### Topic Namespace Pattern- LEDService: Hardware interface for LED control

All topics follow: `led/{pin}/<subtopic>`

## Network Topics

### Subscribes To

- `led/{pin}/command` - Control commands (on, off, toggle, blink)### Subscribed Topics

- `led/status`: Direct status control commands

### Publishes- `capture/start`: Triggers fast blinking during capture

- `led/{pin}/state` - Current state (0=off, 1=on)- `capture/stop`: Sets LED to solid on when capture ends

- `led/{pin}/mode` - Operating mode (0=manual, 1=blinking)- `bluetooth/connected`: Slow blinking when Bluetooth connects

- `bluetooth/disconnected`: Turns LED off when Bluetooth disconnects

## 🔑 API

### Published Topics

### Constructor- `led/status_changed`: Notifies other applications of status changes



```cpp## LED States

LED(int pin);  // Create LED controller for specific GPIO pin

```- **OFF**: LED completely off

- **ON**: LED solid on

### Commands- **BLINKING**: LED blinking at configured interval



Send commands via Network Layer:## Status Commands



```cppStatus can be controlled by sending a single byte to `led/status`:

// Turn LED on- `0`: OFF

const char* cmd = "on";- `1`: ON

networkLayer->publish("led/2/command", (uint8_t*)cmd, strlen(cmd), "MyApp");- `2`: BLINKING (default 500ms interval)



// Turn LED off## Automatic Behaviors

const char* cmd = "off";

networkLayer->publish("led/2/command", (uint8_t*)cmd, strlen(cmd), "MyApp");- **Capture Start**: Fast blink (200ms interval)

- **Capture Stop**: Solid on

// Toggle LED- **Bluetooth Connect**: Slow blink (1000ms interval)

const char* cmd = "toggle";- **Bluetooth Disconnect**: Off

networkLayer->publish("led/2/command", (uint8_t*)cmd, strlen(cmd), "MyApp");

## Usage

// Blink LED

const char* cmd = "blink";The LED application is instantiated and managed in `main.cpp`:

networkLayer->publish("led/2/command", (uint8_t*)cmd, strlen(cmd), "MyApp");

``````cpp

LED* ledApp = new LED(appManager.getNetworkLayer(), appManager.getDataLayer());

## 🚀 Usage Exampleif (!ledApp->setup()) {

    // Handle setup failure

```cpp}

// In main.cpp```

ledApp = new LED(2);  // GPIO 2

ledApp->setNetworkLayer(networkLayer)->setDataLayer(dataLayer);In the main loop:

ledApp->setup();```cpp

ledApp->createTask("LEDApp", 4096, 2, tskNO_AFFINITY, 100);if (ledApp) {

    ledApp->update();

// Control from another application}

networkLayer->subscribe("led/2/state", "MyApp",```

    [](const uint8_t* data, size_t len, const std::string& topic) {

        uint8_t state = data[0];## Configuration

        Serial.printf("LED state: %s\n", state ? "ON" : "OFF");

    });- **Default blink interval**: 500ms for manual blinking

- **Fast blink (capture)**: 200ms

// Send command- **Slow blink (connected)**: 1000ms

const char* cmd = "blink";- **Status persistence**: Stored in Data Layer under "led/current_status"
networkLayer->publish("led/2/command", (uint8_t*)cmd, 5, "MyApp");
```

## 🎛️ Command Reference

| Command | Action | Mode Change |
|---------|--------|-------------|
| `on` | Turn LED on | Manual |
| `off` | Turn LED off | Manual |
| `toggle` | Toggle current state | Manual |
| `blink` | Start blinking (500ms interval) | Blinking |

## 📊 Status Broadcasting

The LED app broadcasts its status periodically (every 1 second):

```cpp
// Publishes to:
led/{pin}/state  → uint8_t (0 or 1)
led/{pin}/mode   → uint8_t (0=manual, 1=blinking)
```

Subscribe to these topics to monitor LED state without polling.

## ⚙️ Configuration

- **Update Frequency**: 100ms (10Hz)
- **Stack Size**: 4096 bytes
- **Priority**: 2
- **Blink Interval**: 500ms

## 🔄 Operation Flow

```
┌─────────────────────────────────────────────┐
│ Command received on led/{pin}/command      │
└─────────────────────────────────────────────┘
                    ↓
          ┌─────────┴─────────┐
          │ Parse command     │
          │ - on/off/toggle   │
          │ - blink           │
          └─────────┬─────────┘
                    ↓
          ┌─────────┴─────────┐
          │ Update GPIO       │
          │ Update mode       │
          └─────────┬─────────┘
                    ↓
┌─────────────────────────────────────────────┐
│ Status broadcast (every 1s)                │
│ → led/{pin}/state                          │
│ → led/{pin}/mode                           │
└─────────────────────────────────────────────┘
```

## 💡 Multiple LED Support

Create separate instances for multiple LEDs:

```cpp
// GPIO 2 - Status LED
ledApp1 = new LED(2);
ledApp1->setNetworkLayer(networkLayer)->setDataLayer(dataLayer);
ledApp1->setup();
ledApp1->createTask("LED_2", 4096, 2, tskNO_AFFINITY, 100);

// GPIO 4 - Alert LED
ledApp2 = new LED(4);
ledApp2->setNetworkLayer(networkLayer)->setDataLayer(dataLayer);
ledApp2->setup();
ledApp2->createTask("LED_4", 4096, 2, tskNO_AFFINITY, 100);

// Control independently
networkLayer->publish("led/2/command", (uint8_t*)"on", 2, "App");
networkLayer->publish("led/4/command", (uint8_t*)"blink", 5, "App");
```

## 📝 Design Philosophy

### Pin-Based Topics
Each LED instance uses its GPIO pin number in topics, enabling:
- Multiple LED support without conflicts
- Clear identification in topic names
- Independent control per LED

### Minimalistic Design
- No complex patterns or effects
- Basic on/off/blink functionality
- Extensible via commands

### Status Broadcasting
- Periodic state publication
- No polling required
- Enables reactive UI/monitoring

## 🔗 Related Applications

- **BluetoothLed** - Uses LED for Bluetooth connection indication
- Any app needing visual feedback

## 🎨 Extension Ideas

Add new commands by extending the command handler:

```cpp
if (cmd == "pulse") {
    // Implement pulse pattern
} else if (cmd == "pattern") {
    // Custom pattern from data
}
```

---

**Part of the TAF Analyzer Application Layer**
