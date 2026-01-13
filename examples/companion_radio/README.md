# Companion Radio

The Companion Radio firmware enables your device to act as a mesh network radio that connects to external applications via USB, BLE, or WiFi.

## Features

- **Serial Interface Support**: Connect via USB, Bluetooth Low Energy (BLE), or WiFi
- **Contact Management**: Store and manage up to 350 contacts
- **Group Channels**: Support for up to 40 group channels
- **Secure Messaging**: End-to-end encrypted communications
- **Optional Repeater Mode**: Enable packet forwarding to extend network coverage

## Firmware Variants

### Standard Companion Firmwares (Non-Repeating)

These firmwares connect to external apps but do NOT forward/repeat packets:

- **`heltec_v4_companion_radio_usb`** - USB serial connection
- **`heltec_v4_companion_radio_ble`** - Bluetooth Low Energy connection
- **`heltec_v4_companion_radio_wifi`** - WiFi TCP connection

### Companion + Repeater Firmwares

These firmwares combine companion functionality WITH packet forwarding (repeater mode):

- **`heltec_v4_companion_repeater_usb`** - USB connection + repeater enabled by default
- **`heltec_v4_companion_repeater_ble`** - BLE connection + repeater enabled by default
- **`heltec_v4_companion_repeater_wifi`** - WiFi connection + repeater enabled by default
- **`heltec_v4_companion_repeater_radio_usb_ble`** - BLE primary interface + USB serial available for debugging + repeater enabled by default (combines everything in one firmware)
- **`heltec_v4_companion_repeater_radio_usb_ble_wifi`** - WiFi primary (up to 3 SSIDs) with BLE fallback + USB serial + repeater enabled
- **`Heltec_v3_companion_repeater_radio_usb_ble_wifi`** - WiFi primary (up to 3 SSIDs) with BLE fallback + USB serial + repeater enabled (Heltec V3)

### Companion Firmwares (Multi-Interface with BLE/WiFi Fallback)

These firmwares support multiple WiFi SSIDs and fall back to BLE if WiFi fails, WITHOUT repeater mode by default:

- **`heltec_v4_companion_radio_usb_ble_wifi`** - WiFi primary (up to 3 SSIDs) with BLE fallback + USB serial (NO repeater by default)
- **`Heltec_v3_companion_radio_usb_ble_wifi`** - WiFi primary (up to 3 SSIDs) with BLE fallback + USB serial (NO repeater by default, Heltec V3)

> **Note**: The `radio_usb_ble` variant uses BLE as the primary companion interface for app connectivity, while USB serial remains available for debugging and monitoring. The `radio_usb_ble_wifi` variants try to connect to WiFi first (supporting up to 3 different SSIDs), and fall back to BLE if all WiFi connections fail. This provides maximum flexibility for deployment in different network environments.

## Repeater Mode

### What is Repeater Mode?

Repeater mode enables your companion device to forward packets through the mesh network, extending coverage and helping messages reach distant nodes. When disabled, the device only sends and receives its own packets.

### Configuration

The repeater functionality is controlled by two settings in `NodePrefs`:

1. **`enable_repeater`** (uint8_t): Enable (1) or disable (0) packet forwarding
2. **`flood_max`** (uint8_t): Maximum number of hops for flood packets (default: 7, range: 1-15)

### Default Behavior

- **Standard companion firmwares**: Repeater mode is **disabled** by default
- **Companion+repeater firmwares**: Repeater mode is **enabled** by default (via `ENABLE_REPEATER_DEFAULT=1` build flag)

### Runtime Control

The repeater mode setting is persisted to flash storage, so it survives reboots. Users can enable or disable repeater mode at runtime through the companion app's device settings (if the app supports this feature).

### Technical Details

The repeater implementation uses:
- `RegionMap` for managing forwarding regions and permissions
- `TransportKeyStore` for key caching
- Region-based flood filtering to prevent packet storms
- Path length limits to prevent excessive hops

When repeater mode is enabled, the `allowPacketForward()` method checks:
1. If repeater mode is enabled (`enable_repeater` flag)
2. If the packet hasn't exceeded the maximum hop count (`flood_max`)
3. If the packet's region permits forwarding (via `RegionMap`)

## Building

To build a specific variant:

```bash
# Standard companion (non-repeating)
FIRMWARE_VERSION="v1.0.0" bash build.sh build-firmware heltec_v4_companion_radio_usb

# Companion with repeater enabled
FIRMWARE_VERSION="v1.0.0" bash build.sh build-firmware heltec_v4_companion_repeater_usb
```

## Connection Details

### USB
- Uses standard serial port (115200 baud by default)
- No PIN required

### BLE
- Device name: `MeshCore-<node_id>`
- PIN: 123456 by default (or random 6-digit PIN if display is available)
- Can be configured via `ble_pin` in NodePrefs

### WiFi
- Connects to WiFi SSID specified in build flags
- Listens on TCP port 5000 by default
- Set `WIFI_SSID` and `WIFI_PWD` in platformio.ini or environment

### Multi-WiFi Support (radio_usb_ble_wifi variants)
The firmware variants with multi-WiFi support can connect to up to 3 different WiFi networks:
- **Primary SSID**: Configured via `WIFI_SSID` and `WIFI_PWD`
- **Secondary SSID**: Configured via `WIFI_SSID2` and `WIFI_PWD2` (optional)
- **Tertiary SSID**: Configured via `WIFI_SSID3` and `WIFI_PWD3` (optional)

The firmware will attempt to connect to each WiFi network in order, using the first one that's available. If all WiFi connections fail, it automatically falls back to BLE mode for connectivity. This is useful for:
- Devices that move between different WiFi networks (home, office, etc.)
- Redundancy in case one WiFi network goes down
- Deployment in environments with multiple WiFi access points

Configure in `platformio.ini`:
```ini
-D WIFI_SSID='"myhome"'
-D WIFI_PWD='"homepassword"'
-D WIFI_SSID2='"myoffice"'
-D WIFI_PWD2='"officepassword"'
-D WIFI_SSID3='"backup"'
-D WIFI_PWD3='"backuppassword"'
```

> **Note**: The secondary and tertiary SSIDs are optional. You can configure just one or two WiFi networks if desired.

## Data Storage

Settings are persisted to flash storage:
- `/new_prefs` - Node preferences including repeater settings
- `/contacts3` - Contact list
- `/channels` - Channel configurations
- `/identity` - Node identity keys

## See Also

- [Simple Repeater](../simple_repeater) - Dedicated repeater firmware without companion features
- [Simple Secure Chat](../simple_secure_chat) - Terminal-based chat application
- [Main README](../../README.md) - Project overview and getting started
