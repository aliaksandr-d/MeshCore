#include <Arduino.h>   // needed for PlatformIO
#include <Mesh.h>
#include "MyMesh.h"

// Believe it or not, this std C function is busted on some platforms!
static uint32_t _atoi(const char* sp) {
  uint32_t n = 0;
  while (*sp && *sp >= '0' && *sp <= '9') {
    n *= 10;
    n += (*sp++ - '0');
  }
  return n;
}

#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
  #include <InternalFileSystem.h>
  #if defined(QSPIFLASH)
    #include <CustomLFS_QSPIFlash.h>
    DataStore store(InternalFS, QSPIFlash, rtc_clock);
  #else
  #if defined(EXTRAFS)
    #include <CustomLFS.h>
    CustomLFS ExtraFS(0xD4000, 0x19000, 128);
    DataStore store(InternalFS, ExtraFS, rtc_clock);
  #else
    DataStore store(InternalFS, rtc_clock);
  #endif
  #endif
#elif defined(RP2040_PLATFORM)
  #include <LittleFS.h>
  DataStore store(LittleFS, rtc_clock);
#elif defined(ESP32)
  #include <SPIFFS.h>
  DataStore store(SPIFFS, rtc_clock);
#endif

#ifdef ESP32
  #if defined(MULTI_WIFI) && defined(BLE_PIN_CODE)
    // Multi-WiFi with BLE fallback support
    #include <helpers/esp32/SerialWifiInterface.h>
    #include <helpers/esp32/SerialBLEInterface.h>
    SerialWifiInterface wifi_interface;
    SerialBLEInterface ble_interface;
    BaseSerialInterface* serial_interface = NULL;  // Will be set based on connection
    #ifndef TCP_PORT
      #define TCP_PORT 5000
    #endif
  #elif defined(WIFI_SSID)
    #include <helpers/esp32/SerialWifiInterface.h>
    SerialWifiInterface serial_interface;
    #ifndef TCP_PORT
      #define TCP_PORT 5000
    #endif
  #elif defined(BLE_PIN_CODE)
    #include <helpers/esp32/SerialBLEInterface.h>
    SerialBLEInterface serial_interface;
  #elif defined(SERIAL_RX)
    #include <helpers/ArduinoSerialInterface.h>
    ArduinoSerialInterface serial_interface;
    HardwareSerial companion_serial(1);
  #else
    #include <helpers/ArduinoSerialInterface.h>
    ArduinoSerialInterface serial_interface;
  #endif
#elif defined(RP2040_PLATFORM)
  //#ifdef WIFI_SSID
  //  #include <helpers/rp2040/SerialWifiInterface.h>
  //  SerialWifiInterface serial_interface;
  //  #ifndef TCP_PORT
  //    #define TCP_PORT 5000
  //  #endif
  // #elif defined(BLE_PIN_CODE)
  //   #include <helpers/rp2040/SerialBLEInterface.h>
  //   SerialBLEInterface serial_interface;
  #if defined(SERIAL_RX)
    #include <helpers/ArduinoSerialInterface.h>
    ArduinoSerialInterface serial_interface;
    HardwareSerial companion_serial(1);
  #else
    #include <helpers/ArduinoSerialInterface.h>
    ArduinoSerialInterface serial_interface;
  #endif
#elif defined(NRF52_PLATFORM)
  #ifdef BLE_PIN_CODE
    #include <helpers/nrf52/SerialBLEInterface.h>
    SerialBLEInterface serial_interface;
  #else
    #include <helpers/ArduinoSerialInterface.h>
    ArduinoSerialInterface serial_interface;
  #endif
#elif defined(STM32_PLATFORM)
  #include <helpers/ArduinoSerialInterface.h>
  ArduinoSerialInterface serial_interface;
#else
  #error "need to define a serial interface"
#endif

/* GLOBAL OBJECTS */
#ifdef DISPLAY_CLASS
  #include "UITask.h"
  #if defined(ESP32) && defined(MULTI_WIFI) && defined(BLE_PIN_CODE)
    UITask ui_task(&board, serial_interface);  // serial_interface is a pointer for MULTI_WIFI
  #else
    UITask ui_task(&board, &serial_interface);
  #endif
#endif

StdRNG fast_rng;
SimpleMeshTables tables;
MyMesh the_mesh(radio_driver, fast_rng, rtc_clock, tables, store
   #ifdef DISPLAY_CLASS
      , &ui_task
   #endif
);

/* END GLOBAL OBJECTS */

void halt() {
  while (1) ;
}

void setup() {
  Serial.begin(115200);

  board.begin();

#ifdef DISPLAY_CLASS
  DisplayDriver* disp = NULL;
  if (display.begin()) {
    disp = &display;
    disp->startFrame();
  #ifdef ST7789
    disp->setTextSize(2);
  #endif
    disp->drawTextCentered(disp->width() / 2, 28, "Loading...");
    disp->endFrame();
  }
#endif

  if (!radio_init()) { halt(); }

  fast_rng.begin(radio_get_rng_seed());

#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
  InternalFS.begin();
  #if defined(QSPIFLASH)
    if (!QSPIFlash.begin()) {
      // debug output might not be available at this point, might be too early. maybe should fall back to InternalFS here?
      MESH_DEBUG_PRINTLN("CustomLFS_QSPIFlash: failed to initialize");
    } else {
      MESH_DEBUG_PRINTLN("CustomLFS_QSPIFlash: initialized successfully");
    }
  #else
  #if defined(EXTRAFS)
      ExtraFS.begin();
  #endif
  #endif
  store.begin();
  the_mesh.begin(
    #ifdef DISPLAY_CLASS
        disp != NULL
    #else
        false
    #endif
  );

#ifdef BLE_PIN_CODE
  char dev_name[32+16];
  sprintf(dev_name, "%s%s", BLE_NAME_PREFIX, the_mesh.getNodeName());
  serial_interface.begin(dev_name, the_mesh.getBLEPin());
#else
  serial_interface.begin(Serial);
#endif
  the_mesh.startInterface(serial_interface);
#elif defined(RP2040_PLATFORM)
  LittleFS.begin();
  store.begin();
  the_mesh.begin(
    #ifdef DISPLAY_CLASS
        disp != NULL
    #else
        false
    #endif
  );

  //#ifdef WIFI_SSID
  //  WiFi.begin(WIFI_SSID, WIFI_PWD);
  //  serial_interface.begin(TCP_PORT);
  // #elif defined(BLE_PIN_CODE)
  //   char dev_name[32+16];
  //   sprintf(dev_name, "%s%s", BLE_NAME_PREFIX, the_mesh.getNodeName());
  //   serial_interface.begin(dev_name, the_mesh.getBLEPin());
  #if defined(SERIAL_RX)
    companion_serial.setPins(SERIAL_RX, SERIAL_TX);
    companion_serial.begin(115200);
    serial_interface.begin(companion_serial);
  #else
    serial_interface.begin(Serial);
  #endif
    the_mesh.startInterface(serial_interface);
#elif defined(ESP32)
  SPIFFS.begin(true);
  store.begin();
  the_mesh.begin(
    #ifdef DISPLAY_CLASS
        disp != NULL
    #else
        false
    #endif
  );

#if defined(MULTI_WIFI) && defined(BLE_PIN_CODE)
  // Multi-WiFi with BLE fallback support
  bool wifi_connected = false;
  
  // Use runtime WiFi credentials from NodePrefs
  NodePrefs* prefs = the_mesh.getNodePrefs();
  
  // Check if we have at least one WiFi SSID configured
  if (prefs->wifi_ssid[0] != '\0') {
    // Check if multi-WiFi is enabled in preferences
    if (prefs->enable_multi_wifi) {
      // Build arrays of SSIDs and passwords from runtime config
      struct WifiCredentials {
        const char* ssid;
        const char* password;
      };
      
      WifiCredentials wifi_credentials[3];
      int num_networks = 0;
      
      // Add configured networks
      if (prefs->wifi_ssid[0] != '\0') {
        wifi_credentials[num_networks].ssid = prefs->wifi_ssid;
        wifi_credentials[num_networks].password = prefs->wifi_pwd;
        num_networks++;
      }
      if (prefs->wifi_ssid2[0] != '\0') {
        wifi_credentials[num_networks].ssid = prefs->wifi_ssid2;
        wifi_credentials[num_networks].password = prefs->wifi_pwd2;
        num_networks++;
      }
      if (prefs->wifi_ssid3[0] != '\0') {
        wifi_credentials[num_networks].ssid = prefs->wifi_ssid3;
        wifi_credentials[num_networks].password = prefs->wifi_pwd3;
        num_networks++;
      }
      
      if (num_networks > 0) {
        Serial.println("Multi-WiFi enabled, attempting WiFi connection...");
        WiFi.mode(WIFI_STA);  // Set WiFi to station mode
        
        for (int i = 0; i < num_networks && !wifi_connected; i++) {
          Serial.print("Trying SSID ");
          Serial.print(i + 1);
          Serial.print("/");
          Serial.print(num_networks);
          Serial.print(": ");
          Serial.println(wifi_credentials[i].ssid);
          
          WiFi.disconnect();
          delay(100);
          WiFi.begin(wifi_credentials[i].ssid, wifi_credentials[i].password);
          
          // Wait up to 15 seconds for connection
          int attempts = 0;
          while (WiFi.status() != WL_CONNECTED && attempts < 30) {
            delay(500);
            Serial.print(".");
            attempts++;
          }
          
          if (WiFi.status() == WL_CONNECTED) {
            wifi_connected = true;
            Serial.println("\nWiFi connected!");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            Serial.print("RSSI: ");
            Serial.println(WiFi.RSSI());
          } else {
            Serial.println("\nFailed to connect");
          }
        }
      }
    } else {
      // Single WiFi mode - only try first SSID
      Serial.println("Single WiFi mode, attempting connection...");
      WiFi.mode(WIFI_STA);
      WiFi.begin(prefs->wifi_ssid, prefs->wifi_pwd);
      
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        wifi_connected = true;
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
      } else {
        Serial.println("\nFailed to connect");
      }
    }
  }
  
  // If WiFi connected, use WiFi interface; otherwise fall back to BLE
  if (wifi_connected) {
    Serial.println("Using WiFi interface");
    wifi_interface.begin(TCP_PORT);
    serial_interface = &wifi_interface;
  } else {
    Serial.println("WiFi failed, falling back to BLE interface");
    char dev_name[32+16];
    sprintf(dev_name, "%s%s", BLE_NAME_PREFIX, the_mesh.getNodeName());
    ble_interface.begin(dev_name, the_mesh.getBLEPin());
    serial_interface = &ble_interface;
  }
  
  serial_interface->enable();
  the_mesh.startInterface(*serial_interface);
#elif defined(WIFI_SSID)
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  serial_interface.begin(TCP_PORT);
  the_mesh.startInterface(serial_interface);
#elif defined(BLE_PIN_CODE)
  char dev_name[32+16];
  sprintf(dev_name, "%s%s", BLE_NAME_PREFIX, the_mesh.getNodeName());
  serial_interface.begin(dev_name, the_mesh.getBLEPin());
  the_mesh.startInterface(serial_interface);
#elif defined(SERIAL_RX)
  companion_serial.setPins(SERIAL_RX, SERIAL_TX);
  companion_serial.begin(115200);
  serial_interface.begin(companion_serial);
  the_mesh.startInterface(serial_interface);
#else
  serial_interface.begin(Serial);
  the_mesh.startInterface(serial_interface);
#endif
#else
  #error "need to define filesystem"
#endif

  sensors.begin();

#ifdef DISPLAY_CLASS
  ui_task.begin(disp, &sensors, the_mesh.getNodePrefs());  // still want to pass this in as dependency, as prefs might be moved
#endif
}

void loop() {
  the_mesh.loop();
  sensors.loop();
#ifdef DISPLAY_CLASS
  ui_task.loop();
#endif
  rtc_clock.tick();
}
