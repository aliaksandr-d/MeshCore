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
