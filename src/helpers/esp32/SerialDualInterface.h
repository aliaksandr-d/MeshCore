#pragma once

#include "../BaseSerialInterface.h"
#include "SerialWifiInterface.h"
#include "SerialBLEInterface.h"

// Dual-mode serial interface that supports both WiFi and BLE
// Allows switching between WiFi and BLE at runtime
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
