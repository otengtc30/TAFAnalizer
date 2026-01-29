#ifndef DATA_LAYER_H
#define DATA_LAYER_H

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

class DataLayer {
public:
    DataLayer();
    ~DataLayer();

    // Initialize with cleanup interval (default 5 seconds)
    bool init(uint32_t cleanupIntervalMs = 5000, UBaseType_t taskPriority = 1, uint32_t taskStackSize = 2048);

    // Basic operations
    bool set(const std::string& key, const std::vector<uint8_t>& value, uint32_t ttlMs = 0);
    bool get(const std::string& key, std::vector<uint8_t>& value);
    bool del(const std::string& key);
    bool exists(const std::string& key);
    std::vector<std::string> keys();

    // TTL operations
    bool expire(const std::string& key, uint32_t ttlMs);
    int32_t ttl(const std::string& key); // Returns remaining TTL in ms, -1 if no TTL, -2 if key doesn't exist

    // Statistics
    size_t size() const;

private:
    struct DataEntry {
        std::vector<uint8_t> value;
        uint32_t expiryTime; // 0 means no expiry
        uint32_t createdTime;
    };

    // RTOS resources
    SemaphoreHandle_t dataMutex_;
    TaskHandle_t cleanupTask_;

    // Data storage
    std::unordered_map<std::string, DataEntry> data_;

    // Configuration
    uint32_t cleanupIntervalMs_;
    bool initialized_;

    // RTOS task function
    static void cleanupTask(void* parameter);

    // Internal cleanup
    void performCleanup();
    uint32_t getCurrentTimeMs();
};

#endif // DATA_LAYER_H