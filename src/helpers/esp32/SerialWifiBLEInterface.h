#pragma once

#include "../BaseSerialInterface.h"
#include "SerialWifiInterface.h"
#include "SerialBLEInterface.h"

// Combined interface that supports both WiFi and BLE simultaneously
class SerialWifiBLEInterface : public BaseSerialInterface {
private:
  SerialWifiInterface wifi_interface;
  SerialBLEInterface ble_interface;
  bool wifi_enabled;
  bool ble_enabled;

public:
  SerialWifiBLEInterface() : wifi_enabled(false), ble_enabled(false) {}

  void beginWifi(int port) {
    wifi_interface.begin(port);
    wifi_enabled = true;
  }

  void beginBLE(const char* device_name, uint32_t pin_code) {
    ble_interface.begin(device_name, pin_code);
    ble_enabled = true;
  }

  // BaseSerialInterface methods
  void enable() override {
    if (wifi_enabled) wifi_interface.enable();
    if (ble_enabled) ble_interface.enable();
  }

  void disable() override {
    if (wifi_enabled) wifi_interface.disable();
    if (ble_enabled) ble_interface.disable();
  }

  bool isEnabled() const override {
    return (wifi_enabled && wifi_interface.isEnabled()) || 
           (ble_enabled && ble_interface.isEnabled());
  }

  bool isConnected() const override {
    return (wifi_enabled && wifi_interface.isConnected()) || 
           (ble_enabled && ble_interface.isConnected());
  }

  bool isWriteBusy() const override {
    // Return true if any interface is busy
    return (wifi_enabled && wifi_interface.isWriteBusy()) || 
           (ble_enabled && ble_interface.isWriteBusy());
  }

  size_t writeFrame(const uint8_t src[], size_t len) override {
    size_t written = 0;
    
    // Try to write to WiFi if connected
    if (wifi_enabled && wifi_interface.isConnected()) {
      written = wifi_interface.writeFrame(src, len);
      if (written > 0) return written;
    }
    
    // Try to write to BLE if connected
    if (ble_enabled && ble_interface.isConnected()) {
      written = ble_interface.writeFrame(src, len);
      if (written > 0) return written;
    }
    
    return 0;
  }

  size_t checkRecvFrame(uint8_t dest[]) override {
    size_t recv = 0;
    
    // Check WiFi for incoming data
    if (wifi_enabled) {
      recv = wifi_interface.checkRecvFrame(dest);
      if (recv > 0) return recv;
    }
    
    // Check BLE for incoming data
    if (ble_enabled) {
      recv = ble_interface.checkRecvFrame(dest);
      if (recv > 0) return recv;
    }
    
    return 0;
  }
};
