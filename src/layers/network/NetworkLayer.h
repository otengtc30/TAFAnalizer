#ifndef NETWORK_LAYER_H
#define NETWORK_LAYER_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class NetworkLayer {
public:
    NetworkLayer();
    ~NetworkLayer();

    // Topic-based message broker API
    using MessageCallback = std::function<void(const uint8_t* data, size_t len, const std::string& topic)>;

    // Initialize the network layer
    bool init();

    // Subscribe to a topic (thread-safe)
    bool subscribe(const std::string& topic, const std::string& appName, MessageCallback callback);

    // Unsubscribe from a topic (thread-safe)
    bool unsubscribe(const std::string& topic, const std::string& appName);

    // Publish a message to a topic (asynchronous - calls subscribers via RTOS task)
    bool publish(const std::string& topic, const uint8_t* data, size_t len, const std::string& publisher = "");

    // Check if topic has subscribers
    bool hasSubscribers(const std::string& topic) const;

    // Get subscriber count for a topic
    size_t getSubscriberCount(const std::string& topic) const;

    // List all available topics
    std::vector<std::string> getTopics() const;

private:
    // Thread-safe subscriber management
    SemaphoreHandle_t subscribersMutex_;
    std::unordered_map<std::string, std::unordered_map<std::string, MessageCallback>> subscribers_;
    bool initialized_;

    // Helper method to deliver message to all subscribers of a topic
    void deliverMessage(const std::string& topic, const uint8_t* data, size_t len);
};

#endif // NETWORK_LAYER_H