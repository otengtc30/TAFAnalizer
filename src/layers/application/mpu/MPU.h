#ifndef MPU_H
#define MPU_H

#include "../ApplicationInterface.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Arduino.h>

// MPU Application
// Handles MPU6050 sensor data collection, processing, and transmission
class MPU : public ApplicationInterface {
public:
    MPU();
    ~MPU();

    bool setup();
    void update();

    // Control methods
    void startCapture();
    void stopCapture();
    bool isCapturing() const;

    // Data access
    bool getLastReading(float& ax, float& ay, float& az, float& gx, float& gy, float& gz) const;

private:
    // MPU6050 hardware
    Adafruit_MPU6050 mpu_;

    // State
    bool initialized_;
    bool capturing_;
    unsigned long lastReadingTime_;
    float last_ax_, last_ay_, last_az_, last_gx_, last_gy_, last_gz_;

    // Configuration
    static const unsigned long READING_INTERVAL_MS = 10; // 100 Hz

    // Network callbacks
    void onStartCapture(const uint8_t* data, size_t len, const std::string& topic);
    void onStopCapture(const uint8_t* data, size_t len, const std::string& topic);
    void onDataRequest(const uint8_t* data, size_t len, const std::string& topic);

    // MPU hardware methods
    bool initMPU();
    bool readMPUData(float& ax, float& ay, float& az, float& gx, float& gy, float& gz);

    // Helper methods
    void readAndPublishData();
    void publishSensorData(float ax, float ay, float az, float gx, float gy, float gz);
    void logSensorData(float ax, float ay, float az, float gx, float gy, float gz);
};

#endif // MPU_H