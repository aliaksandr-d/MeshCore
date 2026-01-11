#pragma once

#include "../BaseSerialInterface.h"
#include "SerialBLEInterface.h"
#include "SerialWifiInterface.h"

// Dual interface supporting both BLE and WiFi connections
class DualSerialInterface : public BaseSerialInterface {
  SerialBLEInterface bleInterface;
  SerialWifiInterface wifiInterface;
  bool _isEnabled;

public:
  DualSerialInterface() {
    _isEnabled = false;
  }

  void beginBLE(const char* device_name, uint32_t pin_code) {
    bleInterface.begin(device_name, pin_code);
  }

  void beginWiFi(SerialWifiInterface::WiFiCredentials* networks, size_t count, int port) {
    wifiInterface.begin(networks, count, port);
  }

  // BaseSerialInterface methods
  void enable() override {
    if (_isEnabled) return;
    _isEnabled = true;
    bleInterface.enable();
    wifiInterface.enable();
  }

  void disable() override {
    _isEnabled = false;
    bleInterface.disable();
    wifiInterface.disable();
  }

  bool isEnabled() const override {
    return _isEnabled;
  }

  bool isConnected() const override {
    return bleInterface.isConnected() || wifiInterface.isConnected();
  }

  bool isWriteBusy() const override {
    // Return true if either interface is busy
    return bleInterface.isWriteBusy() || wifiInterface.isWriteBusy();
  }

  size_t writeFrame(const uint8_t src[], size_t len) override {
    size_t written = 0;
    
    // Try to write to whichever interface is connected
    if (bleInterface.isConnected()) {
      written = bleInterface.writeFrame(src, len);
    }
    
    if (wifiInterface.isConnected()) {
      size_t wifi_written = wifiInterface.writeFrame(src, len);
      // Return max of both writes (both could be connected simultaneously)
      if (wifi_written > written) {
        written = wifi_written;
      }
    }
    
    return written;
  }

  size_t checkRecvFrame(uint8_t dest[]) override {
    // Check BLE first
    size_t len = bleInterface.checkRecvFrame(dest);
    if (len > 0) {
      return len;
    }
    
    // Then check WiFi
    return wifiInterface.checkRecvFrame(dest);
  }
};

#if (BLE_DEBUG_LOGGING || WIFI_DEBUG_LOGGING) && ARDUINO
  #include <Arduino.h>
  #define DUAL_DEBUG_PRINT(F, ...) Serial.printf("Dual: " F, ##__VA_ARGS__)
  #define DUAL_DEBUG_PRINTLN(F, ...) Serial.printf("Dual: " F "\n", ##__VA_ARGS__)
#else
  #define DUAL_DEBUG_PRINT(...) {}
  #define DUAL_DEBUG_PRINTLN(...) {}
#endif
