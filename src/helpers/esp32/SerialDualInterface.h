#pragma once

#include "../BaseSerialInterface.h"
#include "SerialWifiInterface.h"
#include "SerialBLEInterface.h"

/**
 * SerialDualInterface - Dual-mode serial interface supporting both WiFi and BLE
 * 
 * This interface allows a device to offer both WiFi and BLE connectivity simultaneously.
 * Both interfaces are active and can accept connections independently. The implementation
 * handles automatic switching between interfaces based on which one has an active connection
 * and incoming data.
 * 
 * Features:
 * - Both WiFi and BLE interfaces are initialized and enabled simultaneously
 * - Automatic connection priority: the interface that receives data becomes active
 * - Both interfaces can send and receive data independently
 * - Connection state is managed for both interfaces
 * - Users can connect via BLE from mobile devices OR via WiFi from any TCP client
 * 
 * Usage:
 * 1. Call beginWiFi(port) to initialize WiFi server
 * 2. Call beginBLE(device_name, pin_code) to initialize BLE service
 * 3. Call enable() to start both interfaces
 * 
 * Note: On ESP32, WiFi and BLE share the same 2.4GHz radio and use time-division
 * multiplexing managed by the ESP-IDF. While both can be active, there may be
 * slight performance impacts when both are heavily used simultaneously.
 * 
 * This is intended for companion radio firmware where users want the flexibility
 * to connect via either WiFi (e.g., from a desktop app) or BLE (e.g., from a 
 * mobile app) without having to flash different firmware variants.
 */
class SerialDualInterface : public BaseSerialInterface {
private:
  SerialWifiInterface wifiInterface;
  SerialBLEInterface bleInterface;
  bool _wifiEnabled;
  bool _bleEnabled;
  
  // Track which interface was last used for connection
  enum ActiveInterface {
    NONE,
    WIFI,
    BLE
  };
  ActiveInterface activeInterface;

public:
  SerialDualInterface() {
    _wifiEnabled = false;
    _bleEnabled = false;
    activeInterface = NONE;
  }

  void beginWiFi(int port);
  void beginBLE(const char* device_name, uint32_t pin_code);

  // BaseSerialInterface methods
  void enable() override;
  void disable() override;
  bool isEnabled() const override;

  bool isConnected() const override;
  bool isWriteBusy() const override;

  size_t writeFrame(const uint8_t src[], size_t len) override;
  size_t checkRecvFrame(uint8_t dest[]) override;
  
  // Mode switching
  void enableWiFi();
  void enableBLE();
  void disableWiFi();
  void disableBLE();
  
  bool isWiFiEnabled() const { return _wifiEnabled; }
  bool isBLEEnabled() const { return _bleEnabled; }
};

#if (WIFI_DEBUG_LOGGING || BLE_DEBUG_LOGGING) && ARDUINO
  #include <Arduino.h>
  #define DUAL_DEBUG_PRINT(F, ...) Serial.printf("Dual: " F, ##__VA_ARGS__)
  #define DUAL_DEBUG_PRINTLN(F, ...) Serial.printf("Dual: " F "\n", ##__VA_ARGS__)
#else
  #define DUAL_DEBUG_PRINT(...) {}
  #define DUAL_DEBUG_PRINTLN(...) {}
#endif
