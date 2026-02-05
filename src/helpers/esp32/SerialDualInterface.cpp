#include "SerialDualInterface.h"

void SerialDualInterface::beginWiFi(int port) {
  wifiInterface.begin(port);
}

void SerialDualInterface::beginBLE(const char* device_name, uint32_t pin_code) {
  bleInterface.begin(device_name, pin_code);
}

void SerialDualInterface::enable() {
  // Enable both interfaces
  enableWiFi();
  enableBLE();
}

void SerialDualInterface::disable() {
  disableWiFi();
  disableBLE();
}

void SerialDualInterface::enableWiFi() {
  if (!_wifiEnabled) {
    DUAL_DEBUG_PRINTLN("Enabling WiFi interface");
    wifiInterface.enable();
    _wifiEnabled = true;
  }
}

void SerialDualInterface::enableBLE() {
  if (!_bleEnabled) {
    DUAL_DEBUG_PRINTLN("Enabling BLE interface");
    bleInterface.enable();
    _bleEnabled = true;
  }
}

void SerialDualInterface::disableWiFi() {
  if (_wifiEnabled) {
    DUAL_DEBUG_PRINTLN("Disabling WiFi interface");
    wifiInterface.disable();
    _wifiEnabled = false;
    if (activeInterface == WIFI) {
      activeInterface = NONE;
    }
  }
}

void SerialDualInterface::disableBLE() {
  if (_bleEnabled) {
    DUAL_DEBUG_PRINTLN("Disabling BLE interface");
    bleInterface.disable();
    _bleEnabled = false;
    if (activeInterface == BLE) {
      activeInterface = NONE;
    }
  }
}

bool SerialDualInterface::isEnabled() const {
  return _wifiEnabled || _bleEnabled;
}

bool SerialDualInterface::isConnected() const {
  // Return true if either interface is connected
  return (_wifiEnabled && wifiInterface.isConnected()) || 
         (_bleEnabled && bleInterface.isConnected());
}

bool SerialDualInterface::isWriteBusy() const {
  // Check the active interface or both if none is active
  if (activeInterface == WIFI && _wifiEnabled) {
    return wifiInterface.isWriteBusy();
  } else if (activeInterface == BLE && _bleEnabled) {
    return bleInterface.isWriteBusy();
  } else {
    // Check both interfaces
    return (_wifiEnabled && wifiInterface.isWriteBusy()) ||
           (_bleEnabled && bleInterface.isWriteBusy());
  }
}

size_t SerialDualInterface::writeFrame(const uint8_t src[], size_t len) {
  // Try to write to the active interface first
  if (activeInterface == WIFI && _wifiEnabled && wifiInterface.isConnected()) {
    size_t written = wifiInterface.writeFrame(src, len);
    if (written > 0) return written;
  } else if (activeInterface == BLE && _bleEnabled && bleInterface.isConnected()) {
    size_t written = bleInterface.writeFrame(src, len);
    if (written > 0) return written;
  }
  
  // If no active interface, try WiFi first, then BLE
  if (_wifiEnabled && wifiInterface.isConnected()) {
    size_t written = wifiInterface.writeFrame(src, len);
    if (written > 0) {
      activeInterface = WIFI;
      return written;
    }
  }
  
  if (_bleEnabled && bleInterface.isConnected()) {
    size_t written = bleInterface.writeFrame(src, len);
    if (written > 0) {
      activeInterface = BLE;
      return written;
    }
  }
  
  return 0;
}

size_t SerialDualInterface::checkRecvFrame(uint8_t dest[]) {
  size_t len = 0;
  
  // Always call checkRecvFrame on both interfaces to allow them to:
  // 1. Send queued data
  // 2. Manage connection state
  // 3. Check for incoming data
  
  // Check WiFi first
  if (_wifiEnabled) {
    len = wifiInterface.checkRecvFrame(dest);
    if (len > 0) {
      activeInterface = WIFI;
      DUAL_DEBUG_PRINTLN("Received %zu bytes from WiFi", len);
      return len;
    }
  }
  
  // Check BLE
  if (_bleEnabled) {
    len = bleInterface.checkRecvFrame(dest);
    if (len > 0) {
      activeInterface = BLE;
      DUAL_DEBUG_PRINTLN("Received %zu bytes from BLE", len);
      return len;
    }
  }
  
  // Update active interface based on connection status
  if (activeInterface == WIFI && (!_wifiEnabled || !wifiInterface.isConnected())) {
    activeInterface = NONE;
    DUAL_DEBUG_PRINTLN("WiFi disconnected");
  } else if (activeInterface == BLE && (!_bleEnabled || !bleInterface.isConnected())) {
    activeInterface = NONE;
    DUAL_DEBUG_PRINTLN("BLE disconnected");
  }
  
  return 0;
}
