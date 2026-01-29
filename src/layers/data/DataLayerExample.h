#ifndef DATA_LAYER_EXAMPLE_H
#define DATA_LAYER_EXAMPLE_H

#include "DataLayer.h"
#include <Arduino.h>

// Example usage of DataLayer
class DataLayerExample {
public:
    static void demonstrate() {
        Serial.println("\n=== DataLayer Example ===");

        DataLayer dataLayer;

        // Initialize the data layer with 5-second cleanup interval
        if (!dataLayer.init(5000)) {
            Serial.println("Failed to initialize DataLayer");
            return;
        }

        // Example 1: Set and get a simple value
        std::vector<uint8_t> value1 = {'H', 'e', 'l', 'l', 'o'};
        dataLayer.set("greeting", value1);

        std::vector<uint8_t> retrieved1;
        if (dataLayer.get("greeting", retrieved1)) {
            Serial.printf("Retrieved greeting: %.*s\n", retrieved1.size(), retrieved1.data());
        }

        // Example 2: Set a value with TTL (10 seconds)
        std::vector<uint8_t> value2 = {'T', 'e', 'm', 'p', 'o', 'r', 'a', 'r', 'y'};
        dataLayer.set("temp_data", value2, 10000); // 10 seconds TTL

        // Check TTL
        int32_t ttl = dataLayer.ttl("temp_data");
        Serial.printf("TTL for temp_data: %d ms\n", ttl);

        // Example 3: Set expiration on existing key
        dataLayer.expire("greeting", 5000); // 5 seconds

        // Example 4: Check if keys exist
        Serial.printf("Key 'greeting' exists: %s\n", dataLayer.exists("greeting") ? "yes" : "no");
        Serial.printf("Key 'nonexistent' exists: %s\n", dataLayer.exists("nonexistent") ? "yes" : "no");

        // Example 5: List all keys
        auto allKeys = dataLayer.keys();
        Serial.printf("Total keys: %d\n", allKeys.size());
        for (const auto& key : allKeys) {
            Serial.printf("  - %s\n", key.c_str());
        }

        // Example 6: Delete a key
        dataLayer.del("greeting");
        Serial.printf("After deletion, key 'greeting' exists: %s\n", dataLayer.exists("greeting") ? "yes" : "no");

        // Wait a bit to see cleanup in action
        Serial.println("Waiting 6 seconds to see cleanup...");
        delay(6000);

        // Check if temp_data was cleaned up
        Serial.printf("After cleanup, key 'temp_data' exists: %s\n", dataLayer.exists("temp_data") ? "yes" : "no");

        Serial.println("=== DataLayer Example Complete ===");
    }
};

#endif // DATA_LAYER_EXAMPLE_H