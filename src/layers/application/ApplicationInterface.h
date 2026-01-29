#ifndef APPLICATION_INTERFACE_H
#define APPLICATION_INTERFACE_H

#include "../network/NetworkLayer.h"
#include "../data/DataLayer.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class ApplicationInterface {
public:
    ApplicationInterface();
    virtual ~ApplicationInterface();

    // Setup application with layer dependencies
    virtual bool setup() = 0;

    // Update application state (called periodically)
    virtual void update() = 0;

    ApplicationInterface* setNetworkLayer(NetworkLayer* network) {
        networkLayer_ = network;
        return this;
    };
    ApplicationInterface* setDataLayer(DataLayer* data) {
        dataLayer_ = data;
        return this;
    };

    // Task management
    bool createTask(const char* taskName, uint32_t stackSize = 4096, UBaseType_t priority = 2, UBaseType_t coreId = tskNO_AFFINITY, uint32_t updateFrequencyMs = 10);
    void stopTask();
    bool isTaskRunning() const { return taskHandle_ != nullptr; }

protected:
    NetworkLayer* networkLayer_;
    DataLayer* dataLayer_;
    TaskHandle_t taskHandle_;
    uint32_t updateFrequencyMs_;

private:
    // Static task function that calls the virtual update method
    static void taskFunction(void* parameter);
};

#endif // APPLICATION_INTERFACE_H