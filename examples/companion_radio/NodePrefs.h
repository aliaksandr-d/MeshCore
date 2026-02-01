#pragma once
#include <cstdint> // For uint8_t, uint32_t

#define TELEM_MODE_DENY            0
#define TELEM_MODE_ALLOW_FLAGS     1     // use contact.flags
#define TELEM_MODE_ALLOW_ALL       2

#define ADVERT_LOC_NONE       0
#define ADVERT_LOC_SHARE      1

struct NodePrefs {  // persisted to file
  float airtime_factor;
  char node_name[32];
  float freq;
  uint8_t sf;
  uint8_t cr;
  uint8_t multi_acks;
  uint8_t manual_add_contacts;
  float bw;
  uint8_t tx_power_dbm;
  uint8_t telemetry_mode_base;
  uint8_t telemetry_mode_loc;
  uint8_t telemetry_mode_env;
  float rx_delay_base;
  uint32_t ble_pin;
  uint8_t  advert_loc_policy;
  uint8_t  buzzer_quiet;
  uint8_t  enable_repeater;  // enable packet forwarding (repeater mode)
  uint8_t  flood_max;        // max hops for flood packets when repeater enabled
  uint8_t  enable_usb;       // enable USB serial interface (0=disabled, 1=enabled)
  uint8_t  enable_multi_wifi;  // enable multi-WiFi support (0=single SSID, 1=up to 3 SSIDs)
  char wifi_ssid[64];        // Primary WiFi SSID (runtime configurable)
  char wifi_pwd[64];         // Primary WiFi password (runtime configurable)
  char wifi_ssid2[64];       // Secondary WiFi SSID (optional, runtime configurable)
  char wifi_pwd2[64];        // Secondary WiFi password (optional, runtime configurable)
  char wifi_ssid3[64];       // Tertiary WiFi SSID (optional, runtime configurable)
  char wifi_pwd3[64];        // Tertiary WiFi password (optional, runtime configurable)
  uint8_t enable_ping_command;  // enable automatic ping response (0=disabled, 1=enabled)
  char ping_command_channels[128];  // comma-separated list of channels to monitor for ping (e.g., "#test,#bot")
};