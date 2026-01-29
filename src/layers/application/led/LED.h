#ifndef LED_H
#define LED_H

#include "../ApplicationInterface.h"
#include <Arduino.h>
#include <string>

// LED Application
// Minimalistic LED control with topic-based commands and status broadcasting
class LED : public ApplicationInterface {
public:
    LED(uint8_t pin);
    ~LED();

    bool setup();
    void update();

    // Configuration
    void setBroadcastInterval(uint32_t intervalMs);

private:
    // LED configuration
    uint8_t pin_;
    std::string pinNamespace_;

    // State
    bool initialized_;
    bool ledState_;        // true = ON, false = OFF
    bool blinking_;        // true = blinking mode, false = steady mode
    uint32_t blinkInterval_;
    uint32_t broadcastInterval_;
    unsigned long lastBlinkTime_;
    unsigned long lastBroadcastTime_;

    // Network callbacks
    void onCommand(const uint8_t* data, size_t len, const std::string& topic);
    void onBlinkInterval(const uint8_t* data, size_t len, const std::string& topic);

    // LED hardware methods
    void setLedHigh();
    void setLedLow();
    void toggleLed();

    // Helper methods
    void updateBlinking();
    void broadcastStatus();
    void publishMode();
    void publishState();
    std::string getModeString() const;
};

#endif // LED_H