# WiFi Multi-SSID and WiFi+BLE Configuration

## Overview

The Heltec v3 companion radio firmware now supports:
1. Multiple WiFi SSIDs (connect to any available network from a list)
2. Combined WiFi+BLE mode (accept connections via either WiFi or Bluetooth)

## Multiple WiFi SSIDs

### Configuration

To configure multiple WiFi SSIDs, edit the `build_flags` in `variants/heltec_v3/platformio.ini` for the desired firmware environment:

```ini
[env:Heltec_v3_companion_radio_wifi]
build_flags =
  ...
  -D WIFI_SSID='"network1"'
  -D WIFI_PWD='"password1"'
  -D WIFI_SSID_2='"network2"'
  -D WIFI_PWD_2='"password2"'
  -D WIFI_SSID_3='"network3"'
  -D WIFI_PWD_3='"password3"'
  ...
```

### Features

- Supports up to 5 WiFi networks (WIFI_SSID through WIFI_SSID_5)
- Automatically tries each network in order
- 10-second timeout per network
- Backward compatible with single SSID configuration

### How it works

During startup, the firmware will attempt to connect to each configured WiFi network in order. Once connected to any network, it will start the TCP server on the configured port (default: 5000).

## WiFi+BLE Combined Mode

### Available Firmware Targets

- `Heltec_v3_companion_radio_wifi_ble` - Heltec v3 with display
- `Heltec_WSL3_companion_radio_wifi_ble` - Heltec WSL3 (no display)

### Configuration

The WiFi+BLE firmware combines both WiFi and BLE functionality. Configure WiFi networks as shown above, and the BLE PIN is set via:

```ini
-D BLE_PIN_CODE=123456
```

### Features

- Accept connections via WiFi (TCP) or BLE simultaneously
- WiFi clients connect via TCP on port 5000
- BLE clients connect using the configured PIN code
- Device can be connected via either interface independently
- Multiple WiFi SSID support included

### How it works

Both WiFi and BLE interfaces are initialized during startup:
1. WiFi attempts to connect to configured networks
2. BLE advertising starts with the device name based on the mesh node name
3. Incoming data is checked from both interfaces
4. Outgoing data is sent to whichever interface has an active connection

If both interfaces have active connections, WiFi takes priority for receiving and sending data.

## Usage Examples

### Single WiFi Network (backward compatible)
```ini
-D WIFI_SSID='"MyNetwork"'
-D WIFI_PWD='"MyPassword"'
```

### Multiple WiFi Networks
```ini
-D WIFI_SSID='"HomeNetwork"'
-D WIFI_PWD='"HomePassword"'
-D WIFI_SSID_2='"WorkNetwork"'
-D WIFI_PWD_2='"WorkPassword"'
-D WIFI_SSID_3='"MobileHotspot"'
-D WIFI_PWD_3='"HotspotPassword"'
```

### WiFi+BLE with Multiple Networks
Use the `Heltec_v3_companion_radio_wifi_ble` or `Heltec_WSL3_companion_radio_wifi_ble` environment and configure as shown above.

## Building

To build the firmware:

```bash
# Build WiFi-only with multiple SSID support
pio run -e Heltec_v3_companion_radio_wifi

# Build WiFi+BLE combined
pio run -e Heltec_v3_companion_radio_wifi_ble
```

## Troubleshooting

### WiFi not connecting
- Check that SSID and password are correct
- Ensure the WiFi network is in range
- Check serial output for debug messages (WIFI_DEBUG_LOGGING is enabled by default)
- Verify the WiFi network uses 2.4GHz (ESP32 doesn't support 5GHz)

### BLE not advertising
- Check that BLE_PIN_CODE is set
- Verify serial output for BLE debug messages
- Ensure no other BLE operations are blocking (only one BLE central connection is supported)

### Both WiFi and BLE not working
- Check memory usage - both interfaces require significant RAM
- Ensure the firmware is built with WIFI_BLE_BOTH defined
- Review serial debug output for initialization errors
