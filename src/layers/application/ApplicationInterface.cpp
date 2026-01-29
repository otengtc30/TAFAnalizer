#include "ApplicationInterface.h"
#include <Arduino.h>

ApplicationInterface::ApplicationInterface()
    : networkLayer_(nullptr),
      dataLayer_(nullptr),
      taskHandle_(nullptr),
      updateFrequencyMs_(10) {
}

ApplicationInterface::~ApplicationInterface() {
    stopTask();
}

bool ApplicationInterface::createTask(const char* taskName, uint32_t stackSize, UBaseType_t priority, UBaseType_t coreId, uint32_t updateFrequencyMs) {
    if (taskHandle_ != nullptr) {
        Serial.printf("[ApplicationInterface] Task already exists for %s\n", taskName);
        return false;
    }
    Serial.printf("[ApplicationInterface] Creating task %s\n", taskName);

    // Store the update frequency
    updateFrequencyMs_ = updateFrequencyMs;

    BaseType_t result = xTaskCreate(
        taskFunction,        // Task function
        taskName,            // Task name
        stackSize,           // Stack size
        this,                // Task parameter (this pointer)
        priority,            // Priority
        &taskHandle_         // Task handle
    );

    if (result != pdPASS) {
        Serial.printf("[ApplicationInterface] Failed to create task %s\n", taskName);
        return false;
    }

    Serial.printf("[ApplicationInterface] Task %s created successfully (frequency: %d ms)\n", taskName, updateFrequencyMs);
    return true;
}

void ApplicationInterface::stopTask() {
    if (taskHandle_ != nullptr) {
        vTaskDelete(taskHandle_);
        taskHandle_ = nullptr;
        Serial.println("[ApplicationInterface] Task stopped");
    }
}

void ApplicationInterface::taskFunction(void* parameter) {
    Serial.println("[ApplicationInterface] Task function started");
    ApplicationInterface* app = static_cast<ApplicationInterface*>(parameter);
    if (!app) {
        Serial.println("[ApplicationInterface] Invalid application pointer in task");
        vTaskDelete(nullptr);
        return;
    }
    Serial.println("[ApplicationInterface] Task function running");

    while (true) {
        app->update();
        // Delay based on the configured update frequency
        vTaskDelay(pdMS_TO_TICKS(app->updateFrequencyMs_));
    }
}