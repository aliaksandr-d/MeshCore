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

### Companion + Repeater Firmwares (Simplified with Runtime Configuration)

These firmwares combine companion functionality WITH packet forwarding (repeater mode) and support runtime configuration of all features:

- **`heltec_v4_companion_repeater_radio_usb_ble_wifi`** - Multi-WiFi (up to 3 SSIDs) + BLE fallback + USB serial + repeater enabled by default (Heltec V4)
- **`Heltec_v3_companion_repeater_radio_usb_ble_wifi`** - Multi-WiFi (up to 3 SSIDs) + BLE fallback + USB serial + repeater enabled by default (Heltec V3)

> **Note**: These firmwares support **runtime configuration** of repeater, USB, and multi-WiFi features. You can enable/disable these features without reflashing using meshcli commands (see Runtime Configuration section below).

## Runtime Configuration

The companion firmware now supports runtime configuration of key features. These settings are persisted to flash storage and survive reboots.

### Available Parameters

1. **`enable_repeater`** (0 or 1): Enable/disable packet forwarding (repeater mode)
2. **`flood_max`** (1-15): Maximum number of hops for flood packets when repeater is enabled
3. **`enable_usb`** (0 or 1): Enable/disable USB serial interface
4. **`enable_multi_wifi`** (0 or 1): Enable/disable multi-WiFi support (1=try up to 3 SSIDs, 0=use only first SSID)
5. **`wifi_ssid`** (string, max 63 chars): Primary WiFi SSID (runtime configurable)
6. **`wifi_pwd`** (string, max 63 chars): Primary WiFi password (runtime configurable)
7. **`wifi_ssid2`** (string, max 63 chars): Secondary WiFi SSID (optional, runtime configurable)
8. **`wifi_pwd2`** (string, max 63 chars): Secondary WiFi password (optional, runtime configurable)
9. **`wifi_ssid3`** (string, max 63 chars): Tertiary WiFi SSID (optional, runtime configurable)
10. **`wifi_pwd3`** (string, max 63 chars): Tertiary WiFi password (optional, runtime configurable)
11. **`ble_pin`** (uint32_t): BLE PIN code (runtime configurable)

### Configuring via meshcli

If your device supports meshcli access (check the simple_repeater documentation), you can modify these parameters at runtime:

```bash
# Connect via serial or network
meshcli connect /dev/ttyUSB0  # or meshcli connect <ip_address>

# View current settings
config show

# Enable/disable repeater mode
config set enable_repeater 1   # Enable
config set enable_repeater 0   # Disable

# Set maximum flood hops
config set flood_max 10        # Allow up to 10 hops

# Enable/disable USB serial
config set enable_usb 1        # Enable USB
config set enable_usb 0        # Disable USB

# Enable/disable multi-WiFi
config set enable_multi_wifi 1  # Use all configured SSIDs
config set enable_multi_wifi 0  # Use only first SSID

# Configure WiFi credentials (NEW - runtime configurable!)
config set wifi_ssid "MyNetwork"       # Primary WiFi SSID
config set wifi_pwd "MyPassword"       # Primary WiFi password
config set wifi_ssid2 "MyNetwork2"     # Secondary WiFi SSID (optional)
config set wifi_pwd2 "MyPassword2"     # Secondary WiFi password (optional)
config set wifi_ssid3 "MyNetwork3"     # Tertiary WiFi SSID (optional)
config set wifi_pwd3 "MyPassword3"     # Tertiary WiFi password (optional)

# Configure BLE PIN (NEW - runtime configurable!)
config set ble_pin 123456              # BLE PIN code

# Save settings (they persist across reboots)
config save

# Reboot to apply changes
reboot
```

> **Important**: WiFi credentials and BLE PIN are now stored in flash memory and can be changed without firmware recompilation. The build-time defines (`WIFI_SSID`, `WIFI_PWD`, etc.) in platformio.ini are only used as initial defaults on first boot. After that, the runtime values from NodePrefs are used.

> **Note**: The exact meshcli commands may vary depending on the firmware implementation. If meshcli is not available for companion firmware, these settings can only be configured through the companion app (if supported).

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
