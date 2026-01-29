#include "BluetoothLed.h"
#include <Arduino.h>

BluetoothLed::BluetoothLed()
    : initialized_(false),
      bluetoothConnected_(false),
      blinkingActive_(false),
      blinkInterval_(200),    // Fast blink for state changes
      blinkDuration_(3000),   // Blink for 3 seconds
      blinkStartTime_(0) {
    Serial.println("[BluetoothLed] Created - LED coordinator on GPIO 4");
}

BluetoothLed::~BluetoothLed() {
    if (initialized_) {
        // Unsubscribe from topics
        networkLayer_->unsubscribe("bluetooth/connected", "BluetoothLed");
        networkLayer_->unsubscribe("bluetooth/disconnected", "BluetoothLed");

        // Stop any active blinking
        stopLedBlinking();
        Serial.println("[BluetoothLed] Cleaned up");
    }
}

bool BluetoothLed::setup() {
    if (!networkLayer_ || !dataLayer_) {
        Serial.println("[BluetoothLed] ERROR: Missing layer dependencies");
        return false;
    }

    // Subscribe to Bluetooth connection state topics
    auto connectCallback = [this](const uint8_t* data, size_t len, const std::string& topic) {
        this->onBluetoothConnect(data, len, topic);
    };

    auto disconnectCallback = [this](const uint8_t* data, size_t len, const std::string& topic) {
        this->onBluetoothDisconnect(data, len, topic);
    };

    if (!networkLayer_->subscribe("bluetooth/connected", "BluetoothLed", connectCallback)) {
        Serial.println("[BluetoothLed] Failed to subscribe to bluetooth/connected");
        return false;
    }

    if (!networkLayer_->subscribe("bluetooth/disconnected", "BluetoothLed", disconnectCallback)) {
        Serial.println("[BluetoothLed] Failed to subscribe to bluetooth/disconnected");
        return false;
    }

    initialized_ = true;
    Serial.println("[BluetoothLed] Setup complete - coordinating LED blinking for Bluetooth state changes");
    return true;
}

void BluetoothLed::update() {
    if (!initialized_) {
        return;
    }

    // Update blinking state and stop if duration expired
    if (blinkingActive_) {
        updateBlinkingState();
    }
}

void BluetoothLed::setBlinkInterval(uint32_t intervalMs) {
    blinkInterval_ = intervalMs;
    Serial.printf("[BluetoothLed] Blink interval set to %d ms\n", intervalMs);
}

void BluetoothLed::setBlinkDuration(uint32_t durationMs) {
    blinkDuration_ = durationMs;
    Serial.printf("[BluetoothLed] Blink duration set to %d ms\n", durationMs);
}

// Network callbacks
void BluetoothLed::onBluetoothConnect(const uint8_t* data, size_t len, const std::string& topic) {
    bluetoothConnected_ = true;
    Serial.println("[BluetoothLed] Bluetooth connected - starting LED blinking");
    startLedBlinking();
}

void BluetoothLed::onBluetoothDisconnect(const uint8_t* data, size_t len, const std::string& topic) {
    bluetoothConnected_ = false;
    Serial.println("[BluetoothLed] Bluetooth disconnected - starting LED blinking");
    startLedBlinking();
}

// Coordination methods
void BluetoothLed::startLedBlinking() {
    blinkingActive_ = true;
    blinkStartTime_ = millis();
    // First set the blink interval, then start blinking
    publishBlinkInterval(blinkInterval_);
    publishLedCommand(2); // 2 = BLINK command
}

void BluetoothLed::stopLedBlinking() {
    blinkingActive_ = false;
    // Publish command to LED application to turn off
    publishLedCommand(0); // 0 = OFF command
}

void BluetoothLed::updateBlinkingState() {
    unsigned long currentTime = millis();

    // Check if blink duration has expired
    if (currentTime - blinkStartTime_ >= blinkDuration_) {
        stopLedBlinking();
    }
}

void BluetoothLed::publishLedCommand(uint8_t command) {
    networkLayer_->publish("led/2/command", &command, 1);
}

void BluetoothLed::publishBlinkInterval(uint32_t intervalMs) {
    networkLayer_->publish("led/2/blink_interval", reinterpret_cast<const uint8_t*>(&intervalMs), sizeof(uint32_t));
}