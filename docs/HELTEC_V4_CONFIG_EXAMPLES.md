# Example Configuration for Heltec V4 WiFi/BLE Companion Radio

This file contains example configurations for the new WiFi and WiFi+BLE firmware variants.

## Configuration Location
Edit: `variants/heltec_v4/platformio.ini`

## Example 1: WiFi-Only with Multiple SSIDs

Replace the WIFI_SSID_LIST line in `[env:heltec_v4_companion_radio_wifi]`:

```ini
[env:heltec_v4_companion_radio_wifi]
extends = Heltec_lora32_v4
build_flags =
  ${Heltec_lora32_v4.build_flags}
  -I examples/companion_radio/ui-new
  -D MAX_CONTACTS=350
  -D MAX_GROUP_CHANNELS=40
  -D DISPLAY_CLASS=SSD1306Display
  -D WIFI_DEBUG_LOGGING=1
  ; Replace with your WiFi networks:
  -D WIFI_SSID_LIST='{{"MyHomeWiFi","mypassword123"},{"OfficeWiFi","office_pass"},{"PhoneHotspot","mobile123"}}'
build_src_filter = ${Heltec_lora32_v4.build_src_filter}
  +<helpers/ui/SSD1306Display.cpp>
  +<helpers/ui/MomentaryButton.cpp>
  +<helpers/esp32/*.cpp>
  +<../examples/companion_radio/*.cpp>
  +<../examples/companion_radio/ui-new/*.cpp>
lib_deps =
  ${Heltec_lora32_v4.lib_deps}
  densaugeo/base64 @ ~1.4.0
```

## Example 2: WiFi+BLE Dual Mode

Replace the configuration in `[env:heltec_v4_companion_radio_wifi_ble]`:

```ini
[env:heltec_v4_companion_radio_wifi_ble]
extends = Heltec_lora32_v4
build_flags =
  ${Heltec_lora32_v4.build_flags}
  -I examples/companion_radio/ui-new
  -D MAX_CONTACTS=350
  -D MAX_GROUP_CHANNELS=40
  -D DISPLAY_CLASS=SSD1306Display
  -D BLE_PIN_CODE=123456   ; Change this to your desired PIN
  -D AUTO_SHUTDOWN_MILLIVOLTS=3400
  -D BLE_DEBUG_LOGGING=1
  -D WIFI_DEBUG_LOGGING=1
  -D OFFLINE_QUEUE_SIZE=256
  ; Replace with your WiFi networks:
  -D WIFI_SSID_LIST='{{"MyHomeWiFi","mypassword123"},{"OfficeWiFi","office_pass"}}'
build_src_filter = ${Heltec_lora32_v4.build_src_filter}
  +<helpers/ui/SSD1306Display.cpp>
  +<helpers/ui/MomentaryButton.cpp>
  +<helpers/esp32/*.cpp>
  +<../examples/companion_radio/*.cpp>
  +<../examples/companion_radio/ui-new/*.cpp>
lib_deps =
  ${Heltec_lora32_v4.lib_deps}
  densaugeo/base64 @ ~1.4.0
```

## Example 3: Single SSID (Backward Compatible)

For WiFi-only with a single network:

```ini
[env:heltec_v4_companion_radio_wifi]
extends = Heltec_lora32_v4
build_flags =
  ${Heltec_lora32_v4.build_flags}
  -I examples/companion_radio/ui-new
  -D MAX_CONTACTS=350
  -D MAX_GROUP_CHANNELS=40
  -D DISPLAY_CLASS=SSD1306Display
  -D WIFI_DEBUG_LOGGING=1
  ; Single SSID configuration:
  -D WIFI_SSID='"MyHomeWiFi"'
  -D WIFI_PWD='"mypassword123"'
build_src_filter = ${Heltec_lora32_v4.build_src_filter}
  +<helpers/ui/SSD1306Display.cpp>
  +<helpers/ui/MomentaryButton.cpp>
  +<helpers/esp32/*.cpp>
  +<../examples/companion_radio/*.cpp>
  +<../examples/companion_radio/ui-new/*.cpp>
lib_deps =
  ${Heltec_lora32_v4.lib_deps}
  densaugeo/base64 @ ~1.4.0
```

## Important Notes

1. **Escaping**: Use double quotes inside the string literals
2. **Case Sensitive**: SSID and password are case-sensitive
3. **Special Characters**: If your password contains special characters, you may need to escape them
4. **Network Order**: Device will try networks in the order specified
5. **BLE PIN**: Default is 123456, change it for security

## Building

```bash
# Build WiFi-only firmware
pio run -e heltec_v4_companion_radio_wifi

# Build WiFi+BLE firmware
pio run -e heltec_v4_companion_radio_wifi_ble

# Upload to device
pio run -e heltec_v4_companion_radio_wifi_ble --target upload
```

## Connecting

### Via WiFi
1. Device connects to one of your configured networks
2. Check serial monitor (115200 baud) for IP address
3. Connect to IP address on port 5000

### Via BLE
1. Scan for Bluetooth devices
2. Look for device name starting with configured prefix
3. Pair using PIN code (default: 123456)

## Troubleshooting

### WiFi not connecting
- Enable debug logging: `-D WIFI_DEBUG_LOGGING=1`
- Check serial monitor for scan results
- Verify SSID and password are correct
- Make sure at least one network is in range

### BLE not connecting
- Enable debug logging: `-D BLE_DEBUG_LOGGING=1`
- Verify BLE is enabled on your device
- Check that BLE_PIN_CODE matches
- Make sure Bluetooth is not already paired with another device

### Both not working
- Check that `+<helpers/esp32/*.cpp>` is in build_src_filter
- Verify both WIFI_SSID_LIST and BLE_PIN_CODE are defined
- Check for compilation errors
