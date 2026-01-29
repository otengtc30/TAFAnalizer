#include "Camera.h"
#include <Arduino.h>

Camera::Camera()
    : cameraInitialized_(false),
      initialized_(false),
      capturing_(false),
      cameraWorking_(false),
      captureStartTime_(0) {
    Serial.println("[Camera] Created");
}

Camera::~Camera() {
    if (initialized_) {
        // Unsubscribe from topics
        networkLayer_->unsubscribe("capture/start", "Camera");
        networkLayer_->unsubscribe("capture/stop", "Camera");
        networkLayer_->unsubscribe("camera/status", "Camera");

        // Clear buffers
        clearBuffers();
        Serial.println("[Camera] Cleaned up");
    }
}

bool Camera::setup() {
    if (!networkLayer_ || !dataLayer_) {
        Serial.println("[Camera] ERROR: Missing layer dependencies");
        return false;
    }

    // Initialize camera hardware
    if (!initCamera()) {
        Serial.println("[Camera] WARNING: Camera init failed - continuing without camera functionality");
        Serial.println("[Camera] INFO: System will continue with other sensors only (no video)");
        cameraWorking_ = false;
    } else {
        Serial.println("[Camera] Camera initialized successfully - waiting for sensor stabilization...");
        delay(3000); // Increased delay for camera stabilization

        // Test capture to verify camera is working
        Serial.println("[Camera] Testing camera capture...");
        camera_fb_t* test_fb = captureFrameFromHardware();
        if (test_fb) {
            Serial.printf("[Camera] Test capture successful: %d bytes\n", test_fb->len);
            returnFrameBuffer(test_fb);
            cameraWorking_ = true;
            Serial.println("[Camera] INFO: Camera is working - full TAF analysis available");
        } else {
            Serial.println("[Camera] WARNING: Camera test capture failed - camera hardware issue suspected");
            cameraWorking_ = false;
            Serial.println("[Camera] INFO: Check: 1) Power supply stability 2) Camera module connections 3) Try different ESP32-CAM board");
        }
    }

    // Subscribe to network topics
    auto startCallback = [this](const uint8_t* data, size_t len, const std::string& topic) {
        this->onStartCapture(data, len, topic);
    };

    auto stopCallback = [this](const uint8_t* data, size_t len, const std::string& topic) {
        this->onStopCapture(data, len, topic);
    };

    auto statusCallback = [this](const uint8_t* data, size_t len, const std::string& topic) {
        this->onStatusRequest(data, len, topic);
    };

    if (!networkLayer_->subscribe("capture/start", "Camera", startCallback)) {
        Serial.println("[Camera] Failed to subscribe to capture/start");
        return false;
    }

    if (!networkLayer_->subscribe("capture/stop", "Camera", stopCallback)) {
        Serial.println("[Camera] Failed to subscribe to capture/stop");
        return false;
    }

    if (!networkLayer_->subscribe("camera/status", "Camera", statusCallback)) {
        Serial.println("[Camera] Failed to subscribe to camera/status");
        return false;
    }

    initialized_ = true;
    Serial.println("[Camera] Setup complete");
    return true;
}

void Camera::update() {
    if (!initialized_) {
        return;
    }

    // Capture frames if actively capturing and camera is working
    if (capturing_ && cameraWorking_) {
        static unsigned long lastFrameTime = 0;
        unsigned long currentTime = millis();

        // Capture frame (rate limited by delay in main loop)
        captureFrame();

        // Log progress periodically
        if (bufferedFrames_.size() % 5 == 0 && bufferedFrames_.size() > 0) {
            Serial.printf("[Camera] Buffered %d frames, heap: %d bytes\n",
                          bufferedFrames_.size(), ESP.getFreeHeap());
        }
    }

    // Periodic status logging
    static unsigned long lastStatusTime = 0;
    unsigned long currentTime = millis();
    if (currentTime - lastStatusTime >= 5000) {  // Every 5 seconds
        logCameraStatus();
        lastStatusTime = currentTime;
    }
}

void Camera::startCapture() {
    if (!initialized_ || !cameraWorking_) {
        Serial.println("[Camera] Cannot start capture - not initialized or camera not working");
        return;
    }

    capturing_ = true;
    captureStartTime_ = millis();
    clearBuffers();

    Serial.printf("[Camera] Capture started, free heap: %d bytes\n", ESP.getFreeHeap());

    // Publish capture started event
    std::vector<uint8_t> eventData = {'S', 'T', 'A', 'R', 'T', 'E', 'D'};
    networkLayer_->publish("camera/status", eventData.data(), eventData.size());
}

void Camera::stopCapture() {
    if (!initialized_) {
        return;
    }

    capturing_ = false;

    // Transmit all buffered frames
    Serial.printf("[Camera] Capture stopped - transmitting %d buffered frames\n", bufferedFrames_.size());
    transmitBufferedFrames();

    // Clear buffers after transmission
    clearBuffers();

    // Publish capture ended event
    std::vector<uint8_t> eventData = {'S', 'T', 'O', 'P', 'P', 'E', 'D'};
    networkLayer_->publish("camera/status", eventData.data(), eventData.size());

    Serial.println("[Camera] Buffered frame transmission completed");
}

void Camera::onStartCapture(const uint8_t* data, size_t len, const std::string& topic) {
    Serial.println("[Camera] Received start capture command via network");
    startCapture();
}

void Camera::onStopCapture(const uint8_t* data, size_t len, const std::string& topic) {
    Serial.println("[Camera] Received stop capture command via network");
    stopCapture();
}

void Camera::onStatusRequest(const uint8_t* data, size_t len, const std::string& topic) {
    Serial.println("[Camera] Status request received");

    // Send camera status
    std::vector<uint8_t> statusData;
    statusData.push_back(cameraWorking_ ? '1' : '0');  // Camera working
    statusData.push_back(capturing_ ? '1' : '0');      // Currently capturing
    uint32_t frameCount = bufferedFrames_.size();
    statusData.push_back((frameCount >> 24) & 0xFF);
    statusData.push_back((frameCount >> 16) & 0xFF);
    statusData.push_back((frameCount >> 8) & 0xFF);
    statusData.push_back(frameCount & 0xFF);

    networkLayer_->publish("camera/status", statusData.data(), statusData.size());
}

void Camera::captureFrame() {
    if (!cameraWorking_) {
        return;
    }

    camera_fb_t* fb = captureFrameFromHardware();
    if (fb) {
        unsigned long currentTime = millis();

        // Buffer frame with relative timestamp
        TimedFrame timedFrame;
        timedFrame.data.assign(fb->buf, fb->buf + fb->len);
        timedFrame.timestamp = currentTime - captureStartTime_;
        bufferedFrames_.push_back(timedFrame);

        returnFrameBuffer(fb);

        // Limit buffer size to prevent memory issues
        if (bufferedFrames_.size() > maxBufferSize_) {
            bufferedFrames_.erase(bufferedFrames_.begin());
            Serial.println("[Camera] Frame buffer full, removed oldest frame");
        }
    } else {
        Serial.println("[Camera] Frame capture failed");
    }
}

void Camera::transmitBufferedFrames() {
    if (bufferedFrames_.empty()) {
        return;
    }

    // Send frame count first
    std::vector<uint8_t> countData;
    uint32_t frameCount = bufferedFrames_.size();
    countData.push_back((frameCount >> 24) & 0xFF);
    countData.push_back((frameCount >> 16) & 0xFF);
    countData.push_back((frameCount >> 8) & 0xFF);
    countData.push_back(frameCount & 0xFF);

    networkLayer_->publish("camera/frames/count", countData.data(), countData.size());
    delay(100); // Small delay for transmission

    // Send each frame with timestamp
    for (size_t i = 0; i < bufferedFrames_.size(); i++) {
        const TimedFrame& frame = bufferedFrames_[i];

        // Send frame header with index and timestamp
        std::vector<uint8_t> headerData;
        headerData.push_back((i >> 24) & 0xFF);
        headerData.push_back((i >> 16) & 0xFF);
        headerData.push_back((i >> 8) & 0xFF);
        headerData.push_back(i & 0xFF);

        uint32_t timestamp = frame.timestamp;
        headerData.push_back((timestamp >> 24) & 0xFF);
        headerData.push_back((timestamp >> 16) & 0xFF);
        headerData.push_back((timestamp >> 8) & 0xFF);
        headerData.push_back(timestamp & 0xFF);

        uint32_t dataSize = frame.data.size();
        headerData.push_back((dataSize >> 24) & 0xFF);
        headerData.push_back((dataSize >> 16) & 0xFF);
        headerData.push_back((dataSize >> 8) & 0xFF);
        headerData.push_back(dataSize & 0xFF);

        networkLayer_->publish("camera/frame/header", headerData.data(), headerData.size());
        delay(50); // Allow header to be processed

        // Send frame data in chunks to avoid network MTU limits
        const size_t CHUNK_SIZE = 512;
        for (size_t offset = 0; offset < frame.data.size(); offset += CHUNK_SIZE) {
            size_t chunkSize = min(CHUNK_SIZE, frame.data.size() - offset);
            networkLayer_->publish("camera/frame/data", &frame.data[offset], chunkSize);
            delay(10); // Small delay between chunks
        }

        Serial.printf("[Camera] Transmitted frame %d/%d (%d bytes)\n",
                      i + 1, bufferedFrames_.size(), frame.data.size());
    }

    Serial.printf("[Camera] Transmission complete - %d frames sent\n", bufferedFrames_.size());
}

void Camera::clearBuffers() {
    bufferedFrames_.clear();
}

void Camera::logCameraStatus() {
    if (cameraWorking_) {
        Serial.printf("[Camera] Status - Working: YES, Capturing: %s, Buffered: %d/%d frames\n",
                      capturing_ ? "YES" : "NO", bufferedFrames_.size(), maxBufferSize_);
    } else {
        Serial.println("[Camera] Status - Working: NO (camera hardware issue)");
    }
}

size_t Camera::getBufferedFrameCount() const {
    return bufferedFrames_.size();
}

// Camera hardware implementation
bool Camera::initCamera() {
    if (cameraInitialized_) {
        return true;
    }

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 5;
    config.pin_d1 = 18;
    config.pin_d2 = 19;
    config.pin_d3 = 21;
    config.pin_d4 = 36;
    config.pin_d5 = 39;
    config.pin_d6 = 34;
    config.pin_d7 = 35;
    config.pin_xclk = 0;
    config.pin_pclk = 22;
    config.pin_vsync = 25;
    config.pin_href = 23;
    config.pin_sccb_sda = 26;
    config.pin_sccb_scl = 27;
    config.pin_pwdn = 32;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // Frame size and quality
    config.frame_size = FRAMESIZE_QVGA; // 320x240
    config.jpeg_quality = 12; // 0-63, lower number = higher quality
    config.fb_count = 2; // Number of frame buffers

    // Camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("[Camera] Camera init failed with error 0x%x\n", err);
        return false;
    }

    cameraInitialized_ = true;
    Serial.println("[Camera] Camera hardware initialized successfully");
    return true;
}

camera_fb_t* Camera::captureFrameFromHardware() {
    if (!cameraInitialized_) {
        Serial.println("[Camera] Camera not initialized");
        return nullptr;
    }

    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("[Camera] Camera capture failed");
        return nullptr;
    }

    return fb;
}

void Camera::returnFrameBuffer(camera_fb_t* fb) {
    if (fb) {
        esp_camera_fb_return(fb);
    }
}