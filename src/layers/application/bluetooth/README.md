# Bluetooth Application# Bluetooth Application



A fundamental Bluetooth Serial communication application providing connection management and data transmission.The Bluetooth application handles wireless communication, command processing, and device connectivity for the TAF analysis system.



## 🎯 Purpose## Features



Handles Bluetooth Serial connectivity, monitors connection state, and provides data transmission capabilities. This is a core communication layer without application-specific command logic.- **Bluetooth Communication**: ESP32 Bluetooth Serial connectivity

- **Command Processing**: Handle START, STOP, STATUS commands from connected devices

## 🔌 Hardware- **Connection Management**: Monitor and report connection status

- **Data Transmission**: Send binary data and status updates via Bluetooth

- **ESP32 Built-in Bluetooth** - No external hardware required- **Network Integration**: Publish commands and status to other applications

- **Bluetooth Serial Profile** - Standard SPP for serial communication- **Error Handling**: Graceful handling of connection loss and invalid commands



## 📡 Topics## Architecture



### Subscribes To### Communication Flow

- `bluetooth/transmit` - Data to transmit over Bluetooth connection```

Mobile Device ↔ Bluetooth Service ↔ Command Processing ↔ Network Layer ↔ Other Applications

### Publishes```

- `bluetooth/connected` - Published when Bluetooth connection established (payload: `0x01`)

- `bluetooth/disconnected` - Published when Bluetooth connection lost (payload: `0x00`)### Command Processing

- **START**: Initiates capture session across all applications

## 🔑 API- **STOP**: Terminates capture and triggers data transmission

- **STATUS**: Requests current system status

### Public Methods- **Unknown**: Returns error response for invalid commands



```cpp## API Reference

bool isConnected();                               // Check connection status

void sendData(const String& data);                 // Send text data### Initialization

void sendBinaryData(const uint8_t* data, size_t len); // Send binary data```cpp

String getConnectionStatus();                      // Get status stringBluetooth bluetooth(networkLayer, dataLayer);

```if (!bluetooth.setup()) {

    // Handle initialization failure

## 🚀 Usage Example}

```

```cpp

// In main.cpp### Status Methods

bluetoothApp = new Bluetooth();```cpp

bluetoothApp->setNetworkLayer(networkLayer)->setDataLayer(dataLayer);bool connected = bluetooth.isConnected();

bluetoothApp->setup();String status = bluetooth.getConnectionStatus(); // "CONNECTED", "DISCONNECTED", or "NOT_INITIALIZED"

bluetoothApp->createTask("BluetoothApp", 4096, 2, tskNO_AFFINITY, 50);```



// From another application - request data transmission### Data Transmission

const char* message = "Hello Bluetooth";```cpp

networkLayer->publish("bluetooth/transmit", bluetooth.sendData("Hello World");  // Send text data

                     (uint8_t*)message, bluetooth.sendBinaryData(buffer, size);  // Send binary data

                     strlen(message), ```

                     "MyApp");

## Network Topics

// React to connection state

networkLayer->subscribe("bluetooth/connected", "MyApp",### Subscribed Topics

    [](const uint8_t* data, size_t len, const std::string& topic) {- **`capture/start`**: Start capture session

        Serial.println("Bluetooth device connected!");- **`capture/stop`**: Stop capture session

    });- **`status/request`**: Request status update

```- **`bluetooth/transmit`**: Transmit data via Bluetooth



## ⚙️ Configuration### Published Topics

- **`capture/status`**: Capture session status updates

- **Device Name**: `ESP32-CAM-TAF`- **`device/status`**: Device connection and system status

- **Update Frequency**: 50ms (20Hz)

- **Stack Size**: 4096 bytes## Command Protocol

- **Priority**: 2

### Incoming Commands (from Bluetooth client)

## 🔄 Operation Flow```

START\n     - Begin capture session

```STOP\n      - End capture session and transmit data

┌─────────────────────────────────────────────┐STATUS\n    - Request system status

│ 1. Device connects to ESP32-CAM-TAF        │```

└─────────────────────────────────────────────┘

                    ↓### Outgoing Responses (to Bluetooth client)

┌─────────────────────────────────────────────┐```

│ 2. Bluetooth app detects connection        │CAPTURE_STARTED     - Capture session initiated

│    → Publishes bluetooth/connected         │CAPTURE_STOPPED     - Capture session ended

└─────────────────────────────────────────────┘STATUS:BT=1:CAP=0:TIME=12345  - Status update

                    ↓UNKNOWN_COMMAND     - Invalid command received

┌─────────────────────────────────────────────┐```

│ 3. Other apps can send data via            │

│    bluetooth/transmit topic                │## Usage Examples

└─────────────────────────────────────────────┘

                    ↓### Basic Command Processing

┌─────────────────────────────────────────────┐```cpp

│ 4. Device disconnects                      │// In main loop

│    → Publishes bluetooth/disconnected      │bluetooth.update();  // Processes incoming commands automatically

└─────────────────────────────────────────────┘

```// Send data to connected device

if (bluetooth.isConnected()) {

## 📝 Design Philosophy    bluetooth.sendData("Data ready for transmission");

}

This application follows the **fundamental operations** principle:```



### ✅ Included### Network Integration

- Connection state monitoring```cpp

- Data transmission interface// Start capture via network

- State change eventsnetworkLayer->publish("capture/start", nullptr, 0);



### ❌ Excluded// Bluetooth app will receive this and process it

- Command parsing```

- Application-specific logic

- Status reporting formats### Status Monitoring

- Capture control```cpp

// Check connection status

All application-specific logic should be handled by **coordinator applications** that subscribe to Bluetooth events and publish appropriate commands.if (bluetooth.isConnected()) {

    Serial.println("Device is connected");

## 🔗 Related Applications}



- **BluetoothLed** - Coordinator linking Bluetooth state to LED// Get detailed status

- **Data Transmitters** - Any app needing to send data over BluetoothString status = bluetooth.getConnectionStatus();

```

---

## Dependencies

**Part of the TAF Analyzer Application Layer**

- **NetworkLayer**: For inter-application communication
- **DataLayer**: For configuration storage (future use)
- **BluetoothService**: Low-level Bluetooth hardware interface
- **Arduino Framework**: For string handling and timing

## Performance Considerations

- **Command Latency**: Minimal processing delay for incoming commands
- **Connection Monitoring**: Status checks every 2 seconds
- **Data Transmission**: Direct Bluetooth transmission with no buffering
- **Memory Usage**: Low overhead, primarily string processing

## Troubleshooting

### Connection Issues
- Verify Bluetooth is enabled on client device
- Check ESP32 Bluetooth antenna placement
- Monitor connection status logs
- Ensure proper pairing procedure

### Command Not Recognized
- Commands must end with newline character
- Check for extra whitespace or special characters
- Verify command is in uppercase
- Check serial logs for received command format

### Data Transmission Failures
- Verify device is still connected before sending
- Check Bluetooth MTU limitations for large data
- Monitor transmission success in logs

## Future Enhancements

- **Authentication**: Device pairing and security
- **Binary Commands**: Support for binary command protocol
- **Connection Recovery**: Automatic reconnection on disconnection
- **Multiple Clients**: Support for multiple simultaneous connections
- **Data Compression**: Compress data before Bluetooth transmission</content>
<parameter name="filePath">/home/kaipis/Documents/PlatformIO/Projects/TAFAnalizer/src/layers/application/bluetooth/README.md