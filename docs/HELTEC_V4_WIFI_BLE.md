# Multi-WiFi SSID and WiFi+BLE Support for Heltec V4

This document describes the enhancements made to the heltec_v4 companion radio firmware to support multiple WiFi SSIDs and dual WiFi+BLE connectivity.

## Features

### 1. Multiple WiFi SSID Support
The firmware can now connect to any available WiFi network from a configured list. The device will scan for available networks and connect to the first matching SSID from your list.

### 2. Dual WiFi+BLE Mode
A new firmware variant supports both WiFi and Bluetooth Low Energy (BLE) connections simultaneously, allowing you to connect to the device via either interface.

## Firmware Variants

### heltec_v4_companion_radio_wifi
WiFi-only firmware with multiple SSID support.

**Configuration:**
- For single SSID (backward compatible):
  ```cpp
  -D WIFI_SSID='"myssid"'
  -D WIFI_PWD='"mypwd"'
  ```

- For multiple SSIDs:
  ```cpp
  -D WIFI_SSID_LIST='{{"home_network","home_password"},{"work_network","work_password"},{"mobile_hotspot","hotspot_password"}}'
  ```

### heltec_v4_companion_radio_wifi_ble
Dual-mode firmware supporting both WiFi and BLE connections.

**Configuration:**
```cpp
-D BLE_PIN_CODE=123456
-D WIFI_SSID_LIST='{{"home_network","home_password"},{"work_network","work_password"}}'
```

## How It Works

### Multi-SSID Connection Logic
1. Device scans for available WiFi networks
2. Compares scan results with configured SSIDs
3. Attempts to connect to the first available network from the list
4. If connection fails, tries the next available network
5. Server starts only after successful WiFi connection

### Dual WiFi+BLE Mode
- Both interfaces are initialized during startup
- BLE interface is always available
- WiFi connects to any available network from the list
- You can connect to the device via either:
  - Bluetooth using the BLE PIN code
  - WiFi TCP connection on port 5000 (default)
- Both interfaces can be active simultaneously
- Data received from either interface is processed

## Building the Firmware

### Build WiFi-only firmware:
```bash
pio run -e heltec_v4_companion_radio_wifi
```

### Build WiFi+BLE dual-mode firmware:
```bash
pio run -e heltec_v4_companion_radio_wifi_ble
```

## Customization

### Changing WiFi Networks
Edit the platformio.ini file in `variants/heltec_v4/platformio.ini`:

**For WiFi-only mode (`heltec_v4_companion_radio_wifi`):**
```ini
[env:heltec_v4_companion_radio_wifi]
build_flags =
  ...
  ; Comment out the example and add your own networks
  ;-D WIFI_SSID_LIST='{{"home_network","home_password"},{"work_network","work_password"}}'
  -D WIFI_SSID_LIST='{{"MyHomeWiFi","mypassword"},{"WorkWiFi","workpass"}}'
```

**For dual WiFi+BLE mode (`heltec_v4_companion_radio_wifi_ble`):**
```ini
[env:heltec_v4_companion_radio_wifi_ble]
build_flags =
  ...
  -D BLE_PIN_CODE=123456
  -D WIFI_SSID_LIST='{{"MyHomeWiFi","mypassword"},{"WorkWiFi","workpass"}}'
```

### Important Notes on SSID/Password Format
- Use double quotes inside the array: `"ssid_name"`
- Escape special characters if needed
- Network names and passwords are case-sensitive
- You can list as many networks as needed (memory permitting)
- Networks are tried in the order specified

### Changing BLE PIN Code
```ini
-D BLE_PIN_CODE=123456
```

### Changing TCP Port
```ini
-D TCP_PORT=5000
```

## Connection Priority
In dual mode:
- BLE is checked first for incoming data
- WiFi is checked second
- Outgoing data is sent to all connected interfaces

## Debugging
Enable debug logging to see connection details:
```ini
-D WIFI_DEBUG_LOGGING=1
-D BLE_DEBUG_LOGGING=1
```

Monitor the serial output (115200 baud) to see:
- Available networks found during scan
- Connection attempts and results
- Client connections/disconnections
- IP address assigned

## Technical Details

### Files Modified
- `src/helpers/esp32/SerialWifiInterface.h` - Added multi-SSID support
- `src/helpers/esp32/SerialWifiInterface.cpp` - Implemented network scanning and connection logic
- `src/helpers/esp32/DualSerialInterface.h` - New dual interface supporting both BLE and WiFi
- `examples/companion_radio/main.cpp` - Updated to support new interface modes
- `variants/heltec_v4/platformio.ini` - Added new firmware configurations

### Key Classes
- `SerialWifiInterface` - Handles WiFi TCP connections with multi-SSID support
- `SerialBLEInterface` - Handles Bluetooth Low Energy connections
- `DualSerialInterface` - Combines both WiFi and BLE interfaces

### WiFi Connection Process
1. `WiFi.mode(WIFI_STA)` - Set WiFi to station mode
2. `WiFi.scanNetworks()` - Scan for available networks
3. For each configured SSID, check if it's in scan results
4. `WiFi.begin(ssid, password)` - Attempt connection
5. Wait up to 10 seconds for connection
6. If successful, start TCP server
7. If failed, try next SSID in list

## Notes
- The device will attempt to reconnect if WiFi connection is lost
- BLE connection is independent of WiFi status in dual mode
- Multiple SSIDs allow seamless roaming between different networks
- Network credentials are stored in firmware (not in flash storage)
- First match wins - device connects to the first available network in your list
