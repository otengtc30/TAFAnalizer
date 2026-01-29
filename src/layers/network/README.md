# Network Layer# Network Layer - Topic-Based Message Broker



A thread-safe, topic-based message broker implementing the publish-subscribe pattern for inter-application communication.The Network Layer implements a lightweight, topic-based message broker pattern for modular communication between different application components in the ESP32-CAM TAF Analyzer.



## 🎯 Purpose## Architecture



The Network Layer provides a central communication hub that enables loose coupling between applications. Applications can publish messages to topics and subscribe to receive messages from topics without knowing about each other.The Network Layer acts as a central message broker that allows different "apps" (components) to communicate through named topics without direct dependencies.



## 🏗️ Architecture### Key Features



```- **Topic-based routing**: Messages are published to named topics

┌─────────────────────────────────────────────────────┐- **Decoupled communication**: Publishers and subscribers don't need to know about each other

│              Network Layer                          │- **Multiple subscribers**: Multiple apps can subscribe to the same topic

│                                                     │- **Lightweight**: Designed for embedded systems with limited resources

│  ┌─────────────────────────────────────────┐      │- **Type-safe callbacks**: Uses C++11 lambdas for message handling

│  │     Topic Registry                      │      │

│  │  topic1/ → [app1, app2, ...]           │      │## API Overview

│  │  topic2/ → [app3, app4, ...]           │      │

│  └─────────────────────────────────────────┘      │### Core Classes

│                                                     │

│  ┌─────────────────────────────────────────┐      │#### `NetworkLayer`

│  │  Message Delivery (RTOS Tasks)          │      │The main broker class that manages topics and message routing.

│  │  - Asynchronous                         │      │

│  │  - Thread-safe semaphore protection     │      │**Key Methods:**

│  └─────────────────────────────────────────┘      │- `subscribe(topic, appName, callback)` - Subscribe to a topic

└─────────────────────────────────────────────────────┘- `unsubscribe(topic, appName)` - Unsubscribe from a topic

```- `publish(topic, data, len, publisher)` - Publish a message to a topic

- `hasSubscribers(topic)` - Check if topic has active subscribers

## 🔑 Key Features- `getSubscriberCount(topic)` - Get number of subscribers for a topic

- `getTopics()` - Get list of all active topics

### Thread Safety

- **FreeRTOS Semaphores** protect the subscriber registry#### Message Callbacks

- Safe concurrent access from multiple RTOS tasks```cpp

- Prevents race conditions during subscribe/publish operationsusing MessageCallback = std::function<void(const uint8_t* data, size_t len, const std::string& topic)>;

```

### Asynchronous Delivery

- Messages delivered via dedicated RTOS tasks## Usage Example

- Non-blocking publish operations

- Subscribers called in separate task context### 1. Create Network Layer Instance

```cpp

### Topic-Based RoutingNetworkLayer network;

- Hierarchical topic naming (e.g., `bluetooth/connected`, `led/2/command`)```

- Multiple subscribers per topic

- Wildcard support (future enhancement)### 2. Create Application Components

```cpp

## 📋 API Referenceclass MyApp {

private:

### Initialization    NetworkLayer& network_;

    std::string appName_ = "MyApp";

```cpp

NetworkLayer* network = new NetworkLayer();public:

if (!network->init()) {    MyApp(NetworkLayer& network) : network_(network) {}

    // Handle error

}    void init() {

```        // Subscribe to topics of interest

        network_.subscribe("sensor/data", appName_, [this](const uint8_t* data, size_t len, const std::string& topic) {

### Publishing Messages            this->onSensorData(data, len);

        });

```cpp    }

bool publish(

    const std::string& topic,      // Topic name    void publishData() {

    const uint8_t* data,            // Message payload        uint8_t data[4] = {0x01, 0x02, 0x03, 0x04};

    size_t len,                     // Payload length        network_.publish("myapp/data", data, sizeof(data), appName_);

    const std::string& publisher = "" // Optional publisher name    }

);

```private:

    void onSensorData(const uint8_t* data, size_t len) {

**Example:**        // Handle incoming sensor data

```cpp        Serial.printf("Received %d bytes of sensor data\n", len);

const char* message = "Hello World";    }

network->publish("test/topic", };

                 (const uint8_t*)message, ```

                 strlen(message), 

                 "MyApp");### 3. Initialize and Use

``````cpp

MyApp app(network);

### Subscribing to Topicsapp.init();



```cpp// Publish data

bool subscribe(app.publishData();

    const std::string& topic,       // Topic to subscribe to```

    const std::string& appName,     // Subscriber identifier

    MessageCallback callback        // Callback function## Topic Naming Conventions

);

Use hierarchical topic names with forward slashes:

// Callback signature:- `camera/frame` - Camera frame data

void callback(const uint8_t* data, size_t len, const std::string& topic);- `camera/control` - Camera control commands

```- `sensor/mpu6050` - MPU6050 sensor data

- `bluetooth/status` - Bluetooth connection status

**Example:**- `system/status` - System-wide status requests

```cpp

network->subscribe("test/topic", "MyApp", ## Benefits

    [](const uint8_t* data, size_t len, const std::string& topic) {

        Serial.printf("Received %d bytes on %s\n", len, topic.c_str());1. **Modularity**: Components can be developed independently

    });2. **Testability**: Easy to mock network communication for testing

```3. **Scalability**: New components can be added without modifying existing code

4. **Debugging**: Topic-based logging makes message flow visible

### Unsubscribing5. **Resource Efficiency**: Only processes messages for subscribed topics



```cpp## Implementation Notes

bool unsubscribe(const std::string& topic, const std::string& appName);

```- Uses `std::unordered_map` for O(1) topic lookups

- Thread-safe for single-threaded ESP32 applications

### Query Methods- Memory-efficient with automatic cleanup of empty topics

- Includes debug logging for development and troubleshooting

```cpp

// Check if topic has any subscribers## Integration with TAF Analyzer

bool hasSubscribers(const std::string& topic) const;

The Network Layer enables the TAF Analyzer to have modular components:

// Get number of subscribers for a topic

size_t getSubscriberCount(const std::string& topic) const;- **Camera Service** publishes frames to `camera/frame`

- **MPU6050 Service** publishes sensor data to `sensor/mpu6050`

// Get list of all topics- **Bluetooth Service** subscribes to data topics and publishes status to `bluetooth/status`

std::vector<std::string> getTopics() const;- **Web Interface** can subscribe to all data topics for real-time updates

```

This architecture makes it easy to add new sensors, communication protocols, or analysis modules without modifying existing code.
## 🔒 Thread Safety Implementation

### Semaphore Protection

All access to the subscriber registry is protected:

```cpp
if (xSemaphoreTake(subscribersMutex_, portMAX_DELAY) != pdTRUE) {
    return false;
}

// Critical section - modify subscribers_
subscribers_[topic][appName] = callback;

xSemaphoreGive(subscribersMutex_);
```

### Deadlock Prevention

- **No nested locks** - Direct access to internal data structures instead of calling public methods within locks
- **Early mutex release** - Release semaphore before calling subscriber callbacks
- **Copy-on-iterate** - Create subscriber list copy to avoid issues if callbacks modify subscriptions

## ⚡ Message Delivery Flow

1. **Publish Called**
   ```
   Application → publish() → Create RTOS Task
   ```

2. **Delivery Task Executes**
   ```
   Take Semaphore → Find Subscribers → Copy List → Release Semaphore
   ```

3. **Invoke Callbacks**
   ```
   For each subscriber:
       Call callback(data, len, topic)
   ```

## 🎯 Topic Naming Conventions

### Hierarchical Structure
```
<domain>/<entity>/<action>
```

### Examples
```
bluetooth/connected          # Bluetooth connection established
bluetooth/disconnected       # Bluetooth connection lost
bluetooth/transmit          # Request data transmission via Bluetooth

led/2/command               # Command for LED on GPIO 2
led/2/state                 # Current state of LED on GPIO 2
led/2/mode                  # Operating mode of LED

mpu/accel                   # Acceleration data
mpu/gyro                    # Gyroscope data
mpu/temp                    # Temperature data

capture/start               # Start image capture
capture/stop                # Stop image capture
capture/status              # Capture status updates
```

## 🧪 Usage Patterns

### One-to-Many Broadcast
```cpp
// Publisher
network->publish("sensor/data", data, len, "SensorApp");

// Multiple subscribers
network->subscribe("sensor/data", "LoggerApp", logCallback);
network->subscribe("sensor/data", "DisplayApp", displayCallback);
network->subscribe("sensor/data", "BluetoothApp", transmitCallback);
```

### Command-Response
```cpp
// Command sender
network->subscribe("command/response", "Controller", responseHandler);
network->publish("command/request", cmd, len, "Controller");

// Command handler
network->subscribe("command/request", "Executor", 
    [](const uint8_t* data, size_t len, const std::string& topic) {
        // Process command
        // Send response
        network->publish("command/response", response, respLen, "Executor");
    });
```

### State Broadcasting
```cpp
// State publisher (periodic)
void update() {
    static unsigned long lastBroadcast = 0;
    if (millis() - lastBroadcast > 1000) {
        uint8_t state = getState();
        network->publish("device/state", &state, 1, "Device");
        lastBroadcast = millis();
    }
}

// State subscribers
network->subscribe("device/state", "Monitor", monitorCallback);
```

## ⚠️ Important Notes

### Memory Management
- Message data is **not copied** - ensure data remains valid during publish call
- Subscriber callbacks are invoked **synchronously** within delivery task
- Keep callbacks short to avoid blocking other deliveries

### Error Handling
- Failed semaphore acquisition returns `false`
- No subscribers is **not an error** - publish returns `true`
- Exception handling in callbacks prevents one failure from affecting others

### Performance Considerations
- Each publish creates a new RTOS task (4KB stack)
- Tasks delete themselves after delivery
- Consider message batching for high-frequency data

## 🐛 Debugging

### Common Issues

**Deadlock Prevention:**
- ✅ Never call `getSubscriberCount()` within `subscribe()` while holding mutex
- ✅ Release mutex before invoking callbacks
- ✅ Use `portMAX_DELAY` with timeout fallback for production

**Memory Issues:**
- Check task stack size (default 4096 bytes)
- Monitor available heap with `ESP.getFreeHeap()`
- Limit message payload sizes

## 📊 Performance Metrics

- **Subscribe/Unsubscribe**: < 1ms (with mutex)
- **Publish**: < 1ms (task creation only)
- **Delivery**: Depends on subscriber count and callback complexity
- **Task Creation Overhead**: ~4KB RAM per in-flight message

## 🔗 Related Files

- `NetworkLayer.h` - Header with full API
- `NetworkLayer.cpp` - Implementation
- `../application/README.md` - Application layer documentation

---

**Part of the TAF Analyzer layered architecture**
