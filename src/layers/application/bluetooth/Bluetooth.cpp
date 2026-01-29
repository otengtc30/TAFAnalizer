#include "Bluetooth.h"
#include <Arduino.h>

Bluetooth::Bluetooth()
    : initialized_(false) {
    Serial.println("[Bluetooth] Created");
}

Bluetooth::~Bluetooth() {
    if (initialized_) {
        // Unsubscribe from topics
        networkLayer_->unsubscribe("bluetooth/transmit", "Bluetooth");
        Serial.println("[Bluetooth] Cleaned up");
    }
}

bool Bluetooth::setup() {
    if (!networkLayer_ || !dataLayer_) {
        Serial.println("[Bluetooth] ERROR: Missing layer dependencies");
        return false;
    }

    // Initialize Bluetooth
    SerialBT.begin("ESP32-CAM-TAF");
    Serial.println("[Bluetooth] Bluetooth started. Pair with ESP32-CAM-TAF");

    // Subscribe to transmit topic for sending data over Bluetooth
    auto transmitCallback = [this](const uint8_t* data, size_t len, const std::string& topic) {
        this->onTransmitData(data, len, topic);
    };

    Serial.println("[Bluetooth] Subscribing to bluetooth/transmit topic");

    if (!networkLayer_->subscribe("bluetooth/transmit", "Bluetooth", transmitCallback)) {
        Serial.println("[Bluetooth] Failed to subscribe to bluetooth/transmit");
        return false;
    }

    initialized_ = true;
    Serial.println("[Bluetooth] Setup complete");
    return true;
}

void Bluetooth::update() {
    if (!initialized_) {
        return;
    }
    
    // Monitor and publish connection status changes
    logConnectionStatus();
}bool Bluetooth::isConnected() {
    return SerialBT.connected();
}

void Bluetooth::sendData(const String& data) {
    if (SerialBT.connected()) {
        SerialBT.println(data);
    }
}

void Bluetooth::sendBinaryData(const uint8_t* data, size_t len) {
    if (SerialBT.connected()) {
        SerialBT.write(data, len);
    }
}

String Bluetooth::getConnectionStatus() {
    if (!initialized_) {
        return "NOT_INITIALIZED";
    }
    return SerialBT.connected() ? "CONNECTED" : "DISCONNECTED";
}

void Bluetooth::onTransmitData(const uint8_t* data, size_t len, const std::string& topic) {
    Serial.printf("[Bluetooth] Transmitting %d bytes via Bluetooth\n", len);
    if (SerialBT.connected()) {
        SerialBT.write(data, len);
    } else {
        Serial.println("[Bluetooth] Cannot transmit - not connected");
    }
}

void Bluetooth::logConnectionStatus() {
    static bool lastConnected = false;
    bool currentlyConnected = SerialBT.connected();

    if (currentlyConnected != lastConnected) {
        Serial.printf("[Bluetooth] Connection status changed: %s\n",
                     currentlyConnected ? "CONNECTED" : "DISCONNECTED");

        // Publish connection state change to network
        std::string topic = currentlyConnected ? "bluetooth/connected" : "bluetooth/disconnected";
        uint8_t stateData = currentlyConnected ? 1 : 0;
        networkLayer_->publish(topic, &stateData, 1);

        lastConnected = currentlyConnected;
    }
}