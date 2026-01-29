#include "LED.h"

LED::LED(uint8_t pin)
    : pin_(pin),
      pinNamespace_("led/" + std::to_string(pin)),
      initialized_(false),
      ledState_(false),
      blinking_(false),
      blinkInterval_(500),
      broadcastInterval_(5000),  // Broadcast status every 5 seconds
      lastBlinkTime_(0),
      lastBroadcastTime_(0) {
    // Initialize pin as INPUT (as requested, though unusual for LED)
    pinMode(pin_, INPUT);
    Serial.printf("[LED] Created for GPIO %d with namespace '%s'\n", pin, pinNamespace_.c_str());
}

LED::~LED() {
    if (initialized_) {
        // Unsubscribe from topics
        networkLayer_->unsubscribe(pinNamespace_ + "/command", "LED");
        networkLayer_->unsubscribe(pinNamespace_ + "/blink_interval", "LED");
        Serial.printf("[LED] Cleaned up GPIO %d\n", pin_);
    }
}

bool LED::setup() {
    if (!networkLayer_ || !dataLayer_) {
        Serial.println("[LED] ERROR: Missing layer dependencies");
        return false;
    }

    // Subscribe to pin-namespaced topics
    auto commandCallback = [this](const uint8_t* data, size_t len, const std::string& topic) {
        this->onCommand(data, len, topic);
    };

    auto blinkIntervalCallback = [this](const uint8_t* data, size_t len, const std::string& topic) {
        this->onBlinkInterval(data, len, topic);
    };

    std::string commandTopic = pinNamespace_ + "/command";
    std::string blinkTopic = pinNamespace_ + "/blink_interval";

    if (!networkLayer_->subscribe(commandTopic, "LED", commandCallback)) {
        Serial.printf("[LED] Failed to subscribe to %s\n", commandTopic.c_str());
        return false;
    }

    if (!networkLayer_->subscribe(blinkTopic, "LED", blinkIntervalCallback)) {
        Serial.printf("[LED] Failed to subscribe to %s\n", blinkTopic.c_str());
        return false;
    }

    initialized_ = true;
    Serial.printf("[LED] Setup complete for GPIO %d - listening on %s/* topics\n",
                  pin_, pinNamespace_.c_str());

    // Initial status broadcast
    broadcastStatus();

    return true;
}

void LED::update() {
    if (!initialized_) {
        return;
    }

    // Update blinking if active
    if (blinking_) {
        updateBlinking();
    }

    // Periodic status broadcasting
    unsigned long currentTime = millis();
    if (currentTime - lastBroadcastTime_ >= broadcastInterval_) {
        broadcastStatus();
        lastBroadcastTime_ = currentTime;
    }
}

void LED::setBroadcastInterval(uint32_t intervalMs) {
    broadcastInterval_ = intervalMs;
    Serial.printf("[LED] Broadcast interval set to %d ms for GPIO %d\n", intervalMs, pin_);
}

// Network callbacks
void LED::onCommand(const uint8_t* data, size_t len, const std::string& topic) {
    if (len < 1) {
        Serial.printf("[LED] Invalid command data for GPIO %d\n", pin_);
        return;
    }

    uint8_t command = data[0];
    Serial.printf("[LED] Received command %d for GPIO %d\n", command, pin_);

    switch (command) {
        case 0:  // OFF
            blinking_ = false;
            setLedLow();
            publishState();
            publishMode();
            break;
        case 1:  // ON
            blinking_ = false;
            setLedHigh();
            publishState();
            publishMode();
            break;
        case 2:  // BLINK
            blinking_ = true;
            lastBlinkTime_ = millis();
            publishState();
            publishMode();
            break;
        default:
            Serial.printf("[LED] Unknown command %d for GPIO %d\n", command, pin_);
            break;
    }
}

void LED::onBlinkInterval(const uint8_t* data, size_t len, const std::string& topic) {
    if (len >= sizeof(uint32_t)) {
        uint32_t intervalMs = *reinterpret_cast<const uint32_t*>(data);
        blinkInterval_ = intervalMs;
        Serial.printf("[LED] Blink interval set to %d ms for GPIO %d\n", intervalMs, pin_);
    } else {
        Serial.printf("[LED] Invalid blink interval data for GPIO %d\n", pin_);
    }
}

void LED::updateBlinking() {
    unsigned long currentTime = millis();
    if (currentTime - lastBlinkTime_ >= blinkInterval_) {
        toggleLed();
        lastBlinkTime_ = currentTime;
    }
}

void LED::broadcastStatus() {
    publishState();
    publishMode();
}

void LED::publishMode() {
    std::string modeTopic = pinNamespace_ + "/mode";
    std::string modeStr = getModeString();
    networkLayer_->publish(modeTopic, reinterpret_cast<const uint8_t*>(modeStr.c_str()), modeStr.length());
}

void LED::publishState() {
    std::string stateTopic = pinNamespace_ + "/state";
    uint8_t stateValue = ledState_ ? 1 : 0;
    networkLayer_->publish(stateTopic, &stateValue, 1);
}

std::string LED::getModeString() const {
    return blinking_ ? "blinking" : "steady";
}

// LED hardware methods
void LED::setLedHigh() {
    pinMode(pin_, OUTPUT);  // Ensure pin is OUTPUT when controlling
    digitalWrite(pin_, HIGH);
    ledState_ = true;
}

void LED::setLedLow() {
    pinMode(pin_, OUTPUT);  // Ensure pin is OUTPUT when controlling
    digitalWrite(pin_, LOW);
    ledState_ = false;
}

void LED::toggleLed() {
    pinMode(pin_, OUTPUT);  // Ensure pin is OUTPUT when controlling
    ledState_ = !ledState_;
    digitalWrite(pin_, ledState_ ? HIGH : LOW);
}