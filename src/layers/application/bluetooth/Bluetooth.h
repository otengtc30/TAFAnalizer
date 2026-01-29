#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "../ApplicationInterface.h"
#include <BluetoothSerial.h>
#include <Arduino.h>

// Bluetooth Application
// Handles Bluetooth communication, command processing, and device connectivity
class Bluetooth : public ApplicationInterface {
public:
    Bluetooth();
    ~Bluetooth();

    bool setup();
    void update();

    // Control methods
    bool isConnected();
    void sendData(const String& data);
    void sendBinaryData(const uint8_t* data, size_t len);

    // Status
    String getConnectionStatus();

private:

    // Bluetooth hardware
    BluetoothSerial SerialBT;

    // State
    bool initialized_;

    // Network callbacks
    void onTransmitData(const uint8_t* data, size_t len, const std::string& topic);

    // Helper methods
    void logConnectionStatus();
};

#endif // BLUETOOTH_H