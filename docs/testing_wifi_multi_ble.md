# Testing Guide for WiFi Multi-SSID and WiFi+BLE Firmware

## Overview

This document provides testing instructions for the new WiFi multi-SSID and WiFi+BLE features added to the Heltec v3 companion radio firmware.

## Prerequisites

- Heltec v3 or WSL3 hardware
- PlatformIO installed
- Multiple WiFi networks available for testing (or ability to create hotspots)
- BLE-capable device for testing (smartphone, computer with BLE)
- Serial console for viewing debug output

## Test Scenarios

### 1. Test Single WiFi SSID (Backward Compatibility)

**Firmware:** `Heltec_v3_companion_radio_wifi`

**Configuration:**
```ini
-D WIFI_SSID='"YourNetwork"'
-D WIFI_PWD='"YourPassword"'
```

**Expected Behavior:**
- Device should connect to the specified WiFi network
- TCP server should start on port 5000
- MeshCore app should be able to connect via WiFi

**Test Steps:**
1. Build and flash firmware: `pio run -e Heltec_v3_companion_radio_wifi -t upload`
2. Open serial monitor: `pio device monitor -e Heltec_v3_companion_radio_wifi`
3. Verify WiFi connection messages in serial output
4. Note the IP address displayed
5. Connect MeshCore app to the IP address on port 5000
6. Verify communication works

### 2. Test Multiple WiFi SSIDs

**Firmware:** `Heltec_v3_companion_radio_wifi`

**Configuration:**
```ini
-D WIFI_SSID='"Network1"'
-D WIFI_PWD='"Password1"'
-D WIFI_SSID_2='"Network2"'
-D WIFI_PWD_2='"Password2"'
-D WIFI_SSID_3='"Network3"'
-D WIFI_PWD_3='"Password3"'
```

**Test Cases:**

**Test 2a: All networks available**
- Expected: Device connects to Network1 (first in list)
- Verify in serial output which network was selected

**Test 2b: First network unavailable**
- Turn off or move away from Network1
- Expected: Device connects to Network2
- Verify connection to second network in serial output

**Test 2c: Only last network available**
- Ensure Network1 and Network2 are unavailable
- Expected: Device connects to Network3
- Verify connection to third network in serial output

**Test 2d: No networks available**
- Turn off all configured networks
- Expected: Serial output shows "Failed to connect to any configured network"
- Device should still start but WiFi won't be available

**Test 2e: Network roaming**
- Connect to Network1
- Power cycle the device while in range of Network2 only
- Expected: Device should connect to Network2 on next boot

### 3. Test BLE-Only (Verify No Regression)

**Firmware:** `Heltec_v3_companion_radio_ble`

**Expected Behavior:**
- BLE advertising starts with device name "MeshCore-{NodeName}"
- PIN code 123456 is used for pairing
- MeshCore app can connect via BLE

**Test Steps:**
1. Build and flash firmware: `pio run -e Heltec_v3_companion_radio_ble -t upload`
2. Open serial monitor
3. Verify BLE initialization messages
4. Scan for BLE devices on smartphone/computer
5. Connect to "MeshCore-{NodeName}"
6. Enter PIN 123456 when prompted
7. Verify communication works in MeshCore app

### 4. Test WiFi+BLE Combined (Primary Test)

**Firmware:** `Heltec_v3_companion_radio_wifi_ble`

**Configuration:**
```ini
-D WIFI_SSID='"YourNetwork"'
-D WIFI_PWD='"YourPassword"'
-D BLE_PIN_CODE=123456
```

**Test 4a: WiFi connection**
- Expected: Both WiFi and BLE initialize
- WiFi connects to the network
- Device IP address is displayed
- Connect via MeshCore app using WiFi (IP:5000)
- Verify communication works

**Test 4b: BLE connection**
- Expected: BLE advertising is active
- Connect via MeshCore app using BLE
- Verify communication works

**Test 4c: Both interfaces simultaneously**
- Keep WiFi connected
- Connect a BLE client
- Expected: Both connections work
- WiFi client gets priority for data (as designed)

**Test 4d: WiFi+BLE with multiple SSIDs**
```ini
-D WIFI_SSID='"Network1"'
-D WIFI_PWD='"Password1"'
-D WIFI_SSID_2='"Network2"'
-D WIFI_PWD_2='"Password2"'
-D BLE_PIN_CODE=123456
```
- Expected: Device tries WiFi networks in order
- BLE is available regardless of WiFi status
- If WiFi fails, BLE still works

### 5. WSL3 Variant Testing

**Firmware:** `Heltec_WSL3_companion_radio_wifi_ble`

Same tests as #4, but verify it works without display.

## Monitoring and Debugging

### Serial Debug Output

With `WIFI_DEBUG_LOGGING=1` and `BLE_DEBUG_LOGGING=1`, expect to see:

**WiFi Messages:**
```
WiFiMulti: Trying to connect to 3 networks
WiFiMulti: Attempting to connect to 'Network1'
...
WiFiMulti: Connected to 'Network1'
WiFiMulti: IP address: 192.168.1.100
```

**BLE Messages:**
```
BLE: onPassKeyRequest()
BLE: onAuthenticationComplete() - Success
BLE: onConnect(), conn_id=0, mtu=512
```

### Checking Network Status

Monitor the serial output for:
- WiFi connection status
- IP address assignment
- BLE advertising status
- Client connections

### Common Issues

**WiFi not connecting:**
- Verify SSID and password are correct
- Check signal strength
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)

**BLE not working:**
- Verify BLE_PIN_CODE is set
- Check that BLE is not blocked by WiFi (shouldn't be, but verify)
- Try restarting the device

**Both interfaces not working:**
- Check memory usage (may be too high)
- Verify WIFI_BLE_BOTH is defined in build flags

## Performance Testing

### WiFi Performance
- Measure connection time for single SSID vs multiple SSIDs
- Test failover time when primary network becomes unavailable

### BLE Performance
- Measure pairing time
- Test data throughput

### Combined Mode
- Verify no performance degradation when both are active
- Check memory usage

## Success Criteria

- [ ] Single WiFi SSID works (backward compatible)
- [ ] Multiple WiFi SSIDs connect to first available network
- [ ] BLE-only mode still works correctly
- [ ] WiFi+BLE combined mode initializes both interfaces
- [ ] WiFi+BLE mode accepts connections from either interface
- [ ] Multiple SSID failover works correctly
- [ ] Debug logging provides useful information
- [ ] No memory issues or crashes
- [ ] WSL3 variant works correctly

## Reporting Issues

If you encounter issues, please provide:
1. Firmware target used
2. Configuration (SSID count, BLE enabled, etc.)
3. Serial debug output (with WIFI_DEBUG_LOGGING and BLE_DEBUG_LOGGING enabled)
4. Steps to reproduce
5. Expected vs actual behavior
