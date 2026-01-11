#pragma once

#include <WiFi.h>

// Helper class to connect to multiple WiFi SSIDs
class WiFiMultiConnect {
public:
  struct WiFiCredential {
    const char* ssid;
    const char* password;
  };

  // Try to connect to any available WiFi network from the list
  // Returns true if connected, false otherwise
  static bool connect(const WiFiCredential* credentials, size_t count, unsigned long timeout_ms = 30000) {
    if (count == 0 || credentials == nullptr) {
      return false;
    }

    // Scan for available networks
    int n = WiFi.scanNetworks();
    if (n == 0) {
      #ifdef WIFI_DEBUG_LOGGING
      Serial.println("WiFiMulti: No networks found");
      #endif
      return false;
    }

    #ifdef WIFI_DEBUG_LOGGING
    Serial.printf("WiFiMulti: Found %d networks\n", n);
    #endif

    // Try to connect to each credential in order of priority
    for (size_t i = 0; i < count; i++) {
      const char* ssid = credentials[i].ssid;
      const char* password = credentials[i].password;

      // Check if this SSID is in the scan results
      bool found = false;
      for (int j = 0; j < n; j++) {
        if (WiFi.SSID(j) == String(ssid)) {
          found = true;
          break;
        }
      }

      if (!found) {
        #ifdef WIFI_DEBUG_LOGGING
        Serial.printf("WiFiMulti: SSID '%s' not found in scan\n", ssid);
        #endif
        continue;
      }

      #ifdef WIFI_DEBUG_LOGGING
      Serial.printf("WiFiMulti: Attempting to connect to '%s'\n", ssid);
      #endif

      WiFi.begin(ssid, password);

      unsigned long start = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - start < timeout_ms) {
        delay(500);
      }

      if (WiFi.status() == WL_CONNECTED) {
        #ifdef WIFI_DEBUG_LOGGING
        Serial.printf("WiFiMulti: Connected to '%s'\n", ssid);
        Serial.printf("WiFiMulti: IP address: %s\n", WiFi.localIP().toString().c_str());
        #endif
        return true;
      } else {
        #ifdef WIFI_DEBUG_LOGGING
        Serial.printf("WiFiMulti: Failed to connect to '%s'\n", ssid);
        #endif
        WiFi.disconnect();
      }
    }

    #ifdef WIFI_DEBUG_LOGGING
    Serial.println("WiFiMulti: Failed to connect to any network");
    #endif
    return false;
  }
};
