#ifndef CAMERA_H
#define CAMERA_H

#include "../ApplicationInterface.h"
#include <esp_camera.h>
#include <Arduino.h>
#include <vector>

// Camera Application
// Handles camera capture, buffering, and frame transmission
class Camera : public ApplicationInterface {
public:
    Camera();
    ~Camera();

    bool setup();
    void update();

    // Control methods
    void startCapture();
    void stopCapture();
    bool isWorking() const { return cameraWorking_; }

    // Statistics
    size_t getBufferedFrameCount() const;
    size_t getMaxBufferSize() const { return maxBufferSize_; }

private:
    // Camera hardware state
    bool cameraInitialized_;

    // State
    bool initialized_;
    bool capturing_;
    bool cameraWorking_;
    unsigned long captureStartTime_;

    // Configuration
    const size_t maxBufferSize_ = 50;  // Maximum frames to buffer

    // Data structures
    struct TimedFrame {
        std::vector<uint8_t> data;
        unsigned long timestamp;
    };
    std::vector<TimedFrame> bufferedFrames_;

    // Network callbacks
    void onStartCapture(const uint8_t* data, size_t len, const std::string& topic);
    void onStopCapture(const uint8_t* data, size_t len, const std::string& topic);
    void onStatusRequest(const uint8_t* data, size_t len, const std::string& topic);

    // Camera hardware methods
    bool initCamera();
    camera_fb_t* captureFrameFromHardware();
    void returnFrameBuffer(camera_fb_t* fb);

    // Helper methods
    void captureFrame();
    void transmitBufferedFrames();
    void clearBuffers();
    void logCameraStatus();
};

#endif // CAMERA_H