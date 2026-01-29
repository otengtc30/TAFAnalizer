#ifndef BLUETOOTH_LED_H
#define BLUETOOTH_LED_H

#include "../ApplicationInterface.h"
#include <Arduino.h>

// Bluetooth LED Coordinator Application
// Coordinates LED blinking patterns based on Bluetooth connection state changes
// Communicates with LED application via network topics
class BluetoothLed : public ApplicationInterface {
public:
    BluetoothLed();
    ~BluetoothLed();

    bool setup();
    void update();

    // Configuration
    void setBlinkInterval(uint32_t intervalMs);
    void setBlinkDuration(uint32_t durationMs);

private:
    // State
    bool initialized_;
    bool bluetoothConnected_;
    bool blinkingActive_;
    uint32_t blinkInterval_;
    uint32_t blinkDuration_;
    unsigned long blinkStartTime_;

    // Network callbacks
    void onBluetoothConnect(const uint8_t* data, size_t len, const std::string& topic);
    void onBluetoothDisconnect(const uint8_t* data, size_t len, const std::string& topic);

    // Coordination methods
    void startLedBlinking();
    void stopLedBlinking();
    void updateBlinkingState();

    // Helper methods
    void publishLedCommand(uint8_t command);
    void publishBlinkInterval(uint32_t intervalMs);
};

#endif // BLUETOOTH_LED_H