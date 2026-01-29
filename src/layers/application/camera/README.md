# Camera Application

The Camera application handles ESP32-CAM video capture, buffering, and transmission for TAF (Time-and-Force) analysis.

## Features

- **Camera Control**: Initialize and manage ESP32-CAM module
- **Frame Capture**: Capture video frames during analysis sessions
- **Intelligent Buffering**: Buffer frames with timestamps, limit memory usage
- **Network Transmission**: Send captured frames via Network Layer
- **Error Handling**: Graceful degradation when camera hardware fails
- **Status Reporting**: Real-time status updates and diagnostics

## Architecture

### Data Flow
```
Camera Hardware → Camera Service → Frame Buffering → Network Layer → External Systems
```

### Frame Buffer Management
- **Maximum Frames**: 50 frames to prevent memory exhaustion
- **Timestamping**: Relative timestamps from capture start
- **Memory Safety**: Automatic cleanup of oldest frames when buffer full
- **Chunked Transmission**: Large frames sent in 512-byte chunks

## API Reference

### Initialization
```cpp
Camera camera(networkLayer, dataLayer);
if (!camera.setup()) {
    // Handle initialization failure
}
```

### Control Methods
```cpp
camera.startCapture();  // Begin frame buffering
camera.stopCapture();   // Stop and transmit buffered frames
```

### Status Methods
```cpp
bool working = camera.isWorking();           // Check if camera hardware is functional
size_t buffered = camera.getBufferedFrameCount(); // Get current buffer size
size_t maxBuffer = camera.getMaxBufferSize();     // Get maximum buffer capacity
```

## Network Topics

### Subscribed Topics
- **`capture/start`**: Begin frame capture and buffering
- **`capture/stop`**: Stop capture and transmit buffered frames
- **`camera/status`**: Request camera status information

### Published Topics
- **`camera/status`**: Camera status updates and events
- **`camera/frames/count`**: Number of buffered frames being transmitted
- **`camera/frame/header`**: Frame metadata (index, timestamp, size)
- **`camera/frame/data`**: Frame data chunks

## Configuration

### Buffer Limits
- **Maximum Frames**: 50 (configurable via `maxBufferSize_`)
- **Memory Management**: Automatic cleanup prevents heap exhaustion
- **Frame Rate**: Limited by main loop timing (typically 10 FPS)

### Error Handling
- **Hardware Failure**: Continues operation without video (logs warnings)
- **Memory Issues**: Buffer size limits prevent crashes
- **Network Failure**: Frames remain buffered until transmission possible

## Usage Examples

### Basic Capture Session
```cpp
// Start capture
networkLayer->publish("capture/start", nullptr, 0);

// ... capture period ...

// Stop and transmit
networkLayer->publish("capture/stop", nullptr, 0);
```

### Status Monitoring
```cpp
// Request status
networkLayer->publish("camera/status", nullptr, 0);

// Handle status response on "camera/status" topic
```

### Integration with Main Application
```cpp
// In ApplicationSetup
cameraApp_ = std::unique_ptr<Camera>(new Camera(networkLayer_.get(), dataLayer_.get()));

// In main loop
cameraApp_->update();
```

## Dependencies

- **NetworkLayer**: For inter-application communication
- **DataLayer**: For configuration storage (future use)
- **CameraService**: Low-level camera hardware interface
- **Arduino Framework**: For timing and memory functions

## Performance Considerations

- **Memory Usage**: ~150KB per frame (depends on resolution)
- **CPU Usage**: Minimal when not capturing, moderate during capture
- **Network Bandwidth**: High during frame transmission (chunked to prevent congestion)
- **Frame Rate**: Limited to prevent memory overflow

## Troubleshooting

### Camera Not Working
- Check power supply stability
- Verify camera module connections
- Try different ESP32-CAM board
- Check for physical damage to camera

### Memory Issues
- Reduce `maxBufferSize_` if heap is limited
- Monitor free heap with `ESP.getFreeHeap()`
- Consider shorter capture sessions

### Transmission Problems
- Check Network Layer queue status
- Verify topic subscriptions
- Monitor for dropped packets

## Future Enhancements

- **Resolution Control**: Dynamic resolution adjustment
- **Compression**: JPEG compression for smaller frames
- **Quality Settings**: Adjustable image quality
- **Motion Detection**: Smart capture triggering
- **Storage Integration**: Save frames to SSD when network unavailable</content>
<parameter name="filePath">/home/kaipis/Documents/PlatformIO/Projects/TAFAnalizer/src/layers/application/camera/README.md