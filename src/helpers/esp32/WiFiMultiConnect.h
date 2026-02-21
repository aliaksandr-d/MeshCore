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
  static bool connect(const WiFiCredential* credentials, size_t count, const unsigned long timeout_ms = 10000) {
    if (count == 0 || credentials == nullptr) {
      return false;
    }

    #ifdef WIFI_DEBUG_LOGGING
    Serial.printf("WiFiMulti: Trying to connect to %d networks\n", count);
    #endif

    // Try to connect to each credential in order
    for (size_t i = 0; i < count; i++) {
      const char* ssid = credentials[i].ssid;
      const char* password = credentials[i].password;

      #ifdef WIFI_DEBUG_LOGGING
      Serial.printf("WiFiMulti: Attempting to connect to '%s'\n", ssid);
      #endif

      WiFi.begin(ssid, password);

      unsigned long start = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - start < timeout_ms) {
        delay(500);
        #ifdef WIFI_DEBUG_LOGGING
        Serial.print(".");
        #endif
      }

      #ifdef WIFI_DEBUG_LOGGING
      Serial.println();
      #endif

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
