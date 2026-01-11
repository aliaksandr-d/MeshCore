#include "SerialWifiInterface.h"
#include <WiFi.h>

bool SerialWifiInterface::connectToAvailableNetwork(WiFiCredentials* networks, size_t count) {
  if (count == 0) return false;
  
  WIFI_DEBUG_PRINTLN("Scanning for available networks...");
  int numNetworks = WiFi.scanNetworks();
  WIFI_DEBUG_PRINTLN("Found %d networks", numNetworks);
  
  // Try each configured network against scan results
  for (size_t i = 0; i < count; i++) {
    WIFI_DEBUG_PRINTLN("Checking if '%s' is available...", networks[i].ssid);
    
    for (int j = 0; j < numNetworks; j++) {
      String scanSSID = WiFi.SSID(j);
      if (scanSSID.equals(networks[i].ssid)) {
        WIFI_DEBUG_PRINTLN("Found network '%s', attempting to connect...", networks[i].ssid);
        WiFi.begin(networks[i].ssid, networks[i].password);
        
        // Wait for connection (timeout after 10 seconds)
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 40) {
          delay(250);
          attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
          WIFI_DEBUG_PRINTLN("Connected to '%s'", networks[i].ssid);
          WIFI_DEBUG_PRINTLN("IP address: %s", WiFi.localIP().toString().c_str());
          return true;
        } else {
          WIFI_DEBUG_PRINTLN("Failed to connect to '%s'", networks[i].ssid);
        }
        break;
      }
    }
  }
  
  WIFI_DEBUG_PRINTLN("Could not connect to any configured network");
  return false;
}

void SerialWifiInterface::begin(int port) {
  // wifi setup is handled outside of this class, only starts the server
  server.begin(port);
}

void SerialWifiInterface::begin(WiFiCredentials* networks, size_t count, int port) {
  WiFi.mode(WIFI_STA);
  
  if (connectToAvailableNetwork(networks, count)) {
    server.begin(port);
    WIFI_DEBUG_PRINTLN("WiFi server started on port %d", port);
  } else {
    WIFI_DEBUG_PRINTLN("WiFi initialization failed - server not started");
  }
}

// ---------- public methods
void SerialWifiInterface::enable() { 
  if (_isEnabled) return;

  _isEnabled = true;
  clearBuffers();
}

void SerialWifiInterface::disable() {
  _isEnabled = false;
}

size_t SerialWifiInterface::writeFrame(const uint8_t src[], size_t len) {
  if (len > MAX_FRAME_SIZE) {
    WIFI_DEBUG_PRINTLN("writeFrame(), frame too big, len=%d\n", len);
    return 0;
  }

  if (deviceConnected && len > 0) {
    if (send_queue_len >= FRAME_QUEUE_SIZE) {
      WIFI_DEBUG_PRINTLN("writeFrame(), send_queue is full!");
      return 0;
    }

    send_queue[send_queue_len].len = len;  // add to send queue
    memcpy(send_queue[send_queue_len].buf, src, len);
    send_queue_len++;

    return len;
  }
  return 0;
}

bool SerialWifiInterface::isWriteBusy() const {
  return false;
}

size_t SerialWifiInterface::checkRecvFrame(uint8_t dest[]) {
  // check if new client connected
  auto newClient = server.available();
  if (newClient) {

    // disconnect existing client
    deviceConnected = false;
    client.stop();

    // switch active connection to new client
    client = newClient;
    
  }

  if (client.connected()) {
    if (!deviceConnected) {
      WIFI_DEBUG_PRINTLN("Got connection");
      deviceConnected = true;
    }
  } else {
    if (deviceConnected) {
      deviceConnected = false;
      WIFI_DEBUG_PRINTLN("Disconnected");
    }
  }

  if (deviceConnected) {
    if (send_queue_len > 0) {   // first, check send queue
      
      _last_write = millis();
      int len = send_queue[0].len;

      uint8_t pkt[3+len]; // use same header as serial interface so client can delimit frames
      pkt[0] = '>';
      pkt[1] = (len & 0xFF);  // LSB
      pkt[2] = (len >> 8);    // MSB
      memcpy(&pkt[3], send_queue[0].buf, send_queue[0].len);
      client.write(pkt, 3 + len);
      send_queue_len--;
      for (int i = 0; i < send_queue_len; i++) {   // delete top item from queue
        send_queue[i] = send_queue[i + 1];
      }
    } else {
      int len = client.available();
      if (len > 0) {
        uint8_t buf[MAX_FRAME_SIZE + 4];
        client.readBytes(buf, len);
        memcpy(dest, buf+3, len-3); // remove header (don't even check ... problems are on the other dir)
        return len-3;
      }
    }
  }

  return 0;
}

bool SerialWifiInterface::isConnected() const {
  return deviceConnected;  //pServer != NULL && pServer->getConnectedCount() > 0;
}