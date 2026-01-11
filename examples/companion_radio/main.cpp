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
  #ifdef WIFI_BLE_DUAL_MODE
    // Dual mode: Support both WiFi and BLE
    #include <helpers/esp32/SerialWifiInterface.h>
    #include <helpers/esp32/SerialBLEInterface.h>
    SerialWifiInterface serial_wifi_interface;
    SerialBLEInterface serial_ble_interface;
    bool using_wifi_interface = false;  // Track which interface is active
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
  #ifdef WIFI_BLE_DUAL_MODE
    // In dual mode, use WiFi interface for UITask
    UITask ui_task(&board, &serial_wifi_interface);
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

#ifdef WIFI_SSID
  // Multi-WiFi support: try to connect to available networks
  const int MAX_WIFI_CONNECTION_ATTEMPTS = 20;  // 20 * 500ms = 10 seconds
  bool wifi_connected = false;
  
  // Try primary WiFi first
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  WIFI_DEBUG_PRINTLN("Trying to connect to WiFi: %s", WIFI_SSID);
  
  // Wait for connection (up to 10 seconds)
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < MAX_WIFI_CONNECTION_ATTEMPTS) {
    delay(500);
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifi_connected = true;
    WIFI_DEBUG_PRINTLN("Connected to WiFi: %s", WIFI_SSID);
  }
  
  #ifdef WIFI_SSID_1
  // Try first alternative WiFi if primary failed
  if (!wifi_connected) {
    WIFI_DEBUG_PRINTLN("Primary WiFi failed, trying: %s", WIFI_SSID_1);
    WiFi.begin(WIFI_SSID_1, WIFI_PWD_1);
    attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < MAX_WIFI_CONNECTION_ATTEMPTS) {
      delay(500);
      attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      wifi_connected = true;
      WIFI_DEBUG_PRINTLN("Connected to WiFi: %s", WIFI_SSID_1);
    }
  }
  #endif
  
  #ifdef WIFI_SSID_2
  // Try second alternative WiFi if still not connected
  if (!wifi_connected) {
    WIFI_DEBUG_PRINTLN("Alternative WiFi 1 failed, trying: %s", WIFI_SSID_2);
    WiFi.begin(WIFI_SSID_2, WIFI_PWD_2);
    attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < MAX_WIFI_CONNECTION_ATTEMPTS) {
      delay(500);
      attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      wifi_connected = true;
      WIFI_DEBUG_PRINTLN("Connected to WiFi: %s", WIFI_SSID_2);
    }
  }
  #endif
  
  if (!wifi_connected) {
    WIFI_DEBUG_PRINTLN("Failed to connect to any WiFi network");
  }
  
  #ifdef WIFI_BLE_DUAL_MODE
  // In dual mode, initialize both WiFi and BLE
  // WiFi will be used if connected, otherwise BLE
  serial_wifi_interface.begin(TCP_PORT);
  
  char dev_name[32+16];
  sprintf(dev_name, "%s%s", BLE_NAME_PREFIX, the_mesh.getNodeName());
  serial_ble_interface.begin(dev_name, the_mesh.getBLEPin());
  
  // Start with WiFi interface if connected, otherwise use BLE
  if (wifi_connected) {
    the_mesh.startInterface(serial_wifi_interface);
    using_wifi_interface = true;
    WIFI_DEBUG_PRINTLN("Started with WiFi interface (BLE also available)");
  } else {
    the_mesh.startInterface(serial_ble_interface);
    using_wifi_interface = false;
    BLE_DEBUG_PRINTLN("Started with BLE interface (WiFi also available)");
  }
  #else
  // Single WiFi mode
  serial_interface.begin(TCP_PORT);
  the_mesh.startInterface(serial_interface);
  #endif
  
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
#ifdef WIFI_BLE_DUAL_MODE
  // In dual mode, switch interfaces if current one is not connected
  const unsigned long INTERFACE_SWITCH_CHECK_INTERVAL_MS = 5000;
  static unsigned long last_switch_check = 0;
  
  // Check every 5 seconds if we should switch interfaces
  if (millis() - last_switch_check > INTERFACE_SWITCH_CHECK_INTERVAL_MS) {
    last_switch_check = millis();
    
    bool wifi_available = (WiFi.status() == WL_CONNECTED);
    
    // If we're using WiFi but it's not available, switch to BLE
    if (using_wifi_interface && !wifi_available && !serial_wifi_interface.isConnected()) {
      BLE_DEBUG_PRINTLN("Switching to BLE interface");
      the_mesh.startInterface(serial_ble_interface);
      using_wifi_interface = false;
    }
    // If we're using BLE but WiFi is available and we have no BLE connection, switch to WiFi
    else if (!using_wifi_interface && wifi_available && !serial_ble_interface.isConnected()) {
      WIFI_DEBUG_PRINTLN("Switching to WiFi interface");
      the_mesh.startInterface(serial_wifi_interface);
      using_wifi_interface = true;
    }
  }
#endif

  the_mesh.loop();
  sensors.loop();
#ifdef DISPLAY_CLASS
  ui_task.loop();
#endif
  rtc_clock.tick();
}
