#ifndef NETWORK_LAYER_EXAMPLE_H
#define NETWORK_LAYER_EXAMPLE_H

#include "NetworkLayer.h"
#include <Arduino.h>

// Example usage of the NetworkLayer topic-based broker

class CameraApp {
private:
    NetworkLayer& network_;
    std::string appName_ = "CameraApp";

public:
    CameraApp(NetworkLayer& network) : network_(network) {}

    void init() {
        // Subscribe to camera control commands
        network_.subscribe("camera/control", appName_, [this](const uint8_t* data, size_t len, const std::string& topic) {
            this->onControlCommand(data, len);
        });

        // Subscribe to system status requests
        network_.subscribe("system/status", appName_, [this](const uint8_t* data, size_t len, const std::string& topic) {
            this->onStatusRequest(data, len);
        });

        Serial.println("[CameraApp] Initialized and subscribed to topics");
    }

    void captureAndPublish() {
        // Simulate camera capture
        uint8_t frameData[100] = {0xFF, 0xD8}; // Fake JPEG header
        size_t frameSize = sizeof(frameData);

        // Publish captured frame
        network_.publish("camera/frame", frameData, frameSize, appName_);
    }

private:
    void onControlCommand(const uint8_t* data, size_t len) {
        if (len >= 1) {
            uint8_t command = data[0];
            Serial.printf("[CameraApp] Received control command: 0x%02X\n", command);

            switch (command) {
                case 0x01: // Start capture
                    Serial.println("[CameraApp] Starting capture");
                    break;
                case 0x02: // Stop capture
                    Serial.println("[CameraApp] Stopping capture");
                    break;
                default:
                    Serial.println("[CameraApp] Unknown command");
            }
        }
    }

    void onStatusRequest(const uint8_t* data, size_t len) {
        // Respond with camera status
        uint8_t status[4] = {0x01, 0x00, 0x00, 0x00}; // Camera ready, no errors
        network_.publish("camera/status", status, sizeof(status), appName_);
    }
};

class BluetoothApp {
private:
    NetworkLayer& network_;
    std::string appName_ = "BluetoothApp";

public:
    BluetoothApp(NetworkLayer& network) : network_(network) {}

    void init() {
        // Subscribe to frames for transmission
        network_.subscribe("camera/frame", appName_, [this](const uint8_t* data, size_t len, const std::string& topic) {
            this->onFrameReceived(data, len);
        });

        // Subscribe to status responses
        network_.subscribe("camera/status", appName_, [this](const uint8_t* data, size_t len, const std::string& topic) {
            this->onStatusReceived(data, len);
        });

        Serial.println("[BluetoothApp] Initialized and subscribed to topics");
    }

    void sendControlCommand(uint8_t command) {
        // Send control command to camera
        network_.publish("camera/control", &command, 1, appName_);
    }

    void requestStatus() {
        // Request system status
        uint8_t request = 0x01;
        network_.publish("system/status", &request, 1, appName_);
    }

private:
    void onFrameReceived(const uint8_t* data, size_t len) {
        Serial.printf("[BluetoothApp] Received frame: %d bytes\n", len);
        // Here you would transmit the frame via Bluetooth
        // For demo, just acknowledge
        Serial.println("[BluetoothApp] Frame queued for Bluetooth transmission");
    }

    void onStatusReceived(const uint8_t* data, size_t len) {
        if (len >= 4) {
            bool cameraReady = (data[0] != 0);
            Serial.printf("[BluetoothApp] Camera status: %s\n", cameraReady ? "Ready" : "Not Ready");
        }
    }
};

// Example usage function
void demonstrateNetworkLayer() {
    NetworkLayer network;

    // Create apps
    CameraApp camera(network);
    BluetoothApp bluetooth(network);

    // Initialize apps
    camera.init();
    bluetooth.init();

    // Demonstrate communication
    Serial.println("\n=== Network Layer Demo ===");

    // Check topics
    auto topics = network.getTopics();
    Serial.printf("Available topics: %d\n", topics.size());
    for (const auto& topic : topics) {
        Serial.printf("  - %s (%d subscribers)\n", topic.c_str(), network.getSubscriberCount(topic));
    }

    // Send commands
    Serial.println("\n--- Sending control commands ---");
    bluetooth.sendControlCommand(0x01); // Start capture
    delay(100);
    bluetooth.requestStatus();

    // Simulate camera capture
    Serial.println("\n--- Simulating camera capture ---");
    camera.captureAndPublish();

    Serial.println("\n=== Demo Complete ===");
}

#endif // NETWORK_LAYER_EXAMPLE_H