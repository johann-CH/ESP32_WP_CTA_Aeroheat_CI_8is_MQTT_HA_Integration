/*
  ESP32: WP CTA Aeroheat CI 8is Prod Home Assistant Integration (MQTT)
*/

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <Arduino.h>
#include <ESP32.h>
#include <Preferences.h>

#include <Chrono.h>
#include <WiFi.h>
#include <Time.h>
#include <esp_sntp.h>
#include <UnixTime.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerialPro.h>

#include <MQTT.h>

#include "ESP32_WP_CTA_Aeroheat_CI_8is_MQTT.h"

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#define SERIAL_BAUDRATE 115200      // serial interface bauderate
#define SERIAL_DELAY 200            // serial communication delay [ms]
#define SERIAL_RX_BUFFER_SIZE 2048  // serial RX buffer size [byte]
#define SERIAL_TX_BUFFER_SIZE 2048  // serial TX buffer size [byte]

#define CONNECTION_DELAY 1000            // WiFi / MQTT connection delay [ms]
#define CONNECTION_RETRY_DELAY 1000      // WiFi / MQTT retry connection delay [ms]
#define CONNECTION_DISCONNECT_DELAY 500  // WiFi / MQTT disconnection delay [ms]

#define TCP_COMMUNICATION_DELAY 1000  // WP TCP communication delay [ms]
#define TCP_DISCONNECT_DELAY 500      // WP TCP disconnection delay [ms]
#define TCP_RX_DELAY 50               // WP TCP receive delay [ms]
#define TCP_TX_DELAY 50               // WP TCP transmit delay [ms]

#define MQTT_LOOP_DELAY 10       // MQTT loop communication delay [ms]
#define MQTT_SUBSCRIBE_DELAY 10  // MQTT subscribe communication delay [ms]
#define MQTT_PUBLISH_DELAY 10    // MQTT publish communication delay [ms]

#define MQTT_IO_BUFFER_SIZE 512  // MQTT I/O buffer size [byte]

#define WP_DATA_HANDLE_INTERVAL_CHRONO 10  // chrono wp data handle interval [s]

#define STATE_OUTPUT_REFRESH_INTERVAL_CHRONO 10  // chrono wp + converter states output refresh interval [s]

#define WP_STATE_OFFLINE_CHANGE_LIMIT 5  // wp state "Offline" change limit
#define WP_STATE_OFFLINE_STAGE_LIMIT 10  // wp state "Offline" stage limit

#define DEBUG_OUTPUT  // default serial interface output active

#define OTA_OUTPUT  // over the air serial monitor output active

#define ESP32_RESTART_TIME "06:00"      // ESP32 restart time 06h00
#define ESP32_RESTART_LOCK_DELAY 60000  // ESP32 restart lock delay 60s [ms]
#define ESP32_RESTART_DELAY 5000        // ESP32 restart command delay [ms]
#define ESP32_HALT_DELAY 60000          // ESP32 halt delay 60s [ms]

#define WP_SYSTEM_TIME_ADJUST 0         // enable/disable wp system time adjustment
#define BASE_UNIX_TIMESTAMP 1704106800  // 2024-01-01 12:00:00 PM
#define NTP_WP_TIMESTAMP_DIFF 10        // max. SNTP/WP Timestamp Difference [s]
#define GMT 0                           // Greenwich Mean Time
#define GMT_CET 1                       // (+1 GMT)
#define GMT_CEST 2                      // (+1 GMT / +1h summertime offset)

#define WEB_SERIAL_MAX_OUTPUT_COUNT 10    // max. web serial output count
#define WEB_SERIAL_MAX_OUTPUT_DELAY 1000  // web serial output delay [ms] 

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// instantiate preferences library
Preferences nvs;  // non-volatile storage (NVS)

// instantiate chrono objects
Chrono wpDataHandleChrono(Chrono::SECONDS);
Chrono refreshOutputStateChrono(Chrono::SECONDS);

// instantiate WiFi+SNTP+MQTT objects
WiFiClient wifiConnectionID;
MQTTClient mqttClient(MQTT_IO_BUFFER_SIZE);

// instantiate over the air serial monitor
#ifdef OTA_OUTPUT
  AsyncWebServer serialOTA(80);
#endif

// structure definition include <Time.h>
struct tm timeInfo;

// instantiate unix time object
UnixTime wpUnixTime(GMT);

// WiFi definitions
const char wifiSSID[] = "WLAN_Belp";  // WiFi network SSID
const char wifiPassword[] = "family.ruch@belp";  // WiFi network password

// SNTP definitions
const char *timeZone = "CET-1CEST,M3.5.0,M10.5.0/3";  // Time Zone rule for Europe/Rome including daylight adjustment rules (optional)
const char *sntpTimeServer1 = "ch.pool.ntp.org";
const char *sntpTimeServer2 = "pool.ntp.org";
const char *sntpServername = sntpTimeServer1;
u8_t sntpServerID = 0;
uint32_t sntpUnixTimestamp = 0;
uint32_t wpUnixTimestamp = 0;
uint32_t wpUnixTimestampDiff = 0;
uint16_t timeZoneAdjustment = GMT_CET;
const uint32_t gmtOffset = 3600;        // GMT+1 [s]
const uint16_t daylightOffset = 3600;  // 1h [s]
char *sntpDateTime = "1970-01-01 / 00:00:00";
char *wpDateTime = "1970-01-01 / 00:00:00";
char *sntpTimestamp = "00:00";
String sntpTimeZone = "CET";  // "CET" or "CEST"

// CTA Aeroheat CI 8is Prod definitions
const char wpServerIP[] = "192.168.1.190";
int wpServerPort = 8889;
//const char wpUser[] = "";
//const char wpPassword[] = "999999";
const char wpClientID[] = "wp";

// MQTT definitions
const char mqttServer[] = "192.168.1.150";
int mqttServerPort = 1883;
const char mqttUser[] = "mqtt-admin";
const char mqttPassword[] = "mqtt-admin";
const char mqttClientID[] = "wp_mqtt_prod";
bool mqttSuccess;
bool mqttDisconnectSkip = false;

String mqttTopic;
String mqttTopicID;
String mqttTopicEntity;
String mqttPayload;
unsigned long mqttPayloadData;
unsigned int mqttPayloadInt;
float mqttPayloadFloat;
bool mqttRetained = false;
const unsigned int mqttQoS = 0;

unsigned long tcpTxCounter;
unsigned long tcpRxCounter;

uint16_t wpRegisterBlockHeadElement;
uint16_t wpRegisterBlockElements;
uint16_t wpEntityRegisterAddress;
char *wpEntityDescriptor;
char *wpEntityDataType;
float wpEntityDataFactor;
char *wpEntityUnit;
bool wpEntityActiveMQTT;
uint16_t wpStateID = WP_STATE_OFFLINE;
char *wpState = "Offline";
uint16_t wpShutdownReasonID;
char *wpShutdownReason[5];

// communication definitions
unsigned int wpReconnectionCounter = 0;
const unsigned int wpMaxReconnections = 10;

unsigned int mqttReconnectionCounter = 0;
const unsigned int mqttMaxReconnections = 10;

const unsigned int tcpRxPacketSize = 4;  // tcp receive packet size
const unsigned int tcpRxBufferSize = 2048;
uint8_t tcpRxBuffer[tcpRxBufferSize] = {0};  // wp tcp receive buffer
const unsigned int tcpTxPacketSize = 4;  // tcp transmit packet size
const unsigned int tcpTxBufferSize = 4;

unsigned int wpRxRequestedByteNumber = WP_REGISTER_VALUES_NUMBER_BYTES;
unsigned int tcpRxByteAvailable;
unsigned int tcpRxByteReceived;

uint16_t wpTCPCommandGetParameter = 3004;
uint8_t wpTCPCommandGetParameterSequence[tcpTxBufferSize] = {0x00, 0x00, 0x0b, 0x0bc};  // '3004'
uint16_t wpTCPCommandSetParameter = 3002;
uint8_t wpTCPCommandSetParameterSequence[tcpTxBufferSize] = {0x00, 0x00, 0x0b, 0x0ba};  // '3002'
uint16_t wpTCPCommandExecute = 0;
uint8_t wpTCPCommandExecuteSequence[tcpTxBufferSize] = {0x00, 0x00, 0x00, 0x00};  // '0000'
uint8_t wpTCPSetParameterSystemTimeSequence[tcpTxBufferSize] = {0x00, 0x00, 0x00, 0x89};  // '137'
uint8_t wpTCPSetParameterValueSequence[tcpTxBufferSize];
unsigned long tcpCheckGetValue = 0;
unsigned long tcpCheckExecute = 0;
unsigned long tcpCheckParameters = 0;

bool esp32Halt = 0;
bool connectionsEstablished = 0;
bool wpCommandTransmitted = 0;

// common definitions
char msgString[256];  // message char array

String wpRegisterBlockName;
unsigned int wpRegisterBlockIndex = 0;
unsigned int systenTimeBlockIndex = 0;
unsigned int tcpBufferIndex;

unsigned long wpDataHandleCounter = 0;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  // initialize chronos
  wpDataHandleChrono.stop();
  refreshOutputStateChrono.stop();
  
  // initialize default serial interface
  Serial.setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
  Serial.setTxBufferSize(SERIAL_TX_BUFFER_SIZE);
  Serial.begin(SERIAL_BAUDRATE);
  delay(SERIAL_DELAY);
  Serial.flush();
  
  // initialize NVS (R+W)
  nvs.begin("CTA_NVS", false);
  
  // read persisted NVS values
  for(uint8_t i = 0; i < ESP32_RESTART_INFO_ELEMENTS; i++) {
    esp32RestartInfo[i].esp32RestartCounter = nvs.getUInt(esp32RestartInfo[i].esp32RestartReasonNVS, esp32RestartInfo[i].esp32RestartCounter);
  }
  
  // output infoline
  Serial.println("CTA Aeroheat CI 8is Prod [Init WiFi + MQTT]");
  Serial.println();
  
  // initialize WiFi network connection
  Serial.print("WiFi connecting...");
  WiFi.mode(WIFI_STA);  // WiFi station mode
  WiFi.begin(wifiSSID, wifiPassword);
  delay(CONNECTION_DELAY);
  while(WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.disconnect();
    sprintf(msgString, "\nWiFi Error: WiFi not connected!");
    Serial.println(msgString);
    
    esp32Restart(ESP32_RESTART_WIFI_CONNECTION);
  }
  Serial.print("\nWiFi <");
  Serial.print(wifiSSID);
  Serial.print("> / <");
  Serial.print(WiFi.localIP());
  Serial.println("> connected.\n");
  Serial.println();
  
  // initialize SNTP
  esp_sntp_init();
  esp_sntp_setservername(sntpServerID, sntpTimeServer1);
  configTime(gmtOffset, daylightOffset, sntpTimeServer1, sntpTimeServer2);
  configTzTime(timeZone, sntpTimeServer1, sntpTimeServer2);
  Serial.println("SNTP initialized...\n");
	
  // initialize over the air serial monitor
  #ifdef OTA_OUTPUT
    WebSerial.begin(&serialOTA);
    WebSerial.onMessage(serialOTAReceiver);
    serialOTA.begin();
    WebSerial.setID("WP Prod Home Assistant Integration");
  #endif
  
  // WP server connection only used in loop()
  
  // initialize MQTT server connection
  Serial.print("MQTT server <");
  Serial.print(mqttServer);
  Serial.print("> connecting...");
  mqttClient.begin(mqttServer, mqttServerPort, wifiConnectionID);
  delay(CONNECTION_DELAY);
  mqttReconnectionCounter = 0;
  while(!mqttClient.connected() && (mqttReconnectionCounter < mqttMaxReconnections)) {
    mqttReconnectionCounter++;
    mqttClient.connect(mqttClientID, mqttUser, mqttPassword, mqttDisconnectSkip);
    Serial.print("-");
    delay(CONNECTION_RETRY_DELAY);
  }
  if(mqttReconnectionCounter >= mqttMaxReconnections) {
    sprintf(msgString, "\nMQTT Error: MQTT server not connected after <%u> retries", mqttReconnectionCounter);
    Serial.println(msgString);
    
    esp32Restart(ESP32_RESTART_MQTT_CONNECTION);
  }
  Serial.println("\nMQTT server connected.");
  Serial.println();
  
  if((WiFi.status() == WL_CONNECTED) && mqttClient.connected()) {
    connectionsEstablished = 1;
    
    // MQTT subscriptions
    #ifdef DEBUG_OUTPUT
      Serial.println("CTA Aeroheat CI 8is Prod [MQTT Subscriptions]");
    #endif
  }
  
  // reset counters
  wpReconnectionCounter = 0;
  mqttReconnectionCounter = 0;
  wpRegisterBlockIndex = 0;
  
  // start chronos
  wpDataHandleChrono.start();
  refreshOutputStateChrono.start();
  
  #ifdef DEBUG_OUTPUT
    Serial.println();
  #endif
  
  // output wp states 
  outputInfoline();
}  // end setup()

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop() {
  // handle SNTP
  if(getLocalTime(&timeInfo)) {
    char timeInfoEpoch[16];
    strftime(timeInfoEpoch, 16, "%s", &timeInfo);
    sntpUnixTimestamp = atoi(timeInfoEpoch);
    
    char timeInfoDST[8];
    strftime(timeInfoDST, 8, "%Z", &timeInfo);
    sntpTimeZone = timeInfoDST;
  }
  
  if(sntpUnixTimestamp > BASE_UNIX_TIMESTAMP) {  // sntp unix timestamp validation
    if(sntpTimeZone == "CET") {
      //sntpTimeZone = "CET";
		  timeZoneAdjustment = GMT_CET;  // Greenwich Mean Time +1 (Central European Winter Time)
	  } else {
      //sntpTimeZone = "CEST";
		  timeZoneAdjustment = GMT_CEST;  // Greenwich Mean Time +2 (Central European Summer Time)
	  }
  }
  
  #ifdef ESP32_RESTART_TIME
  // restart only if ESP32_RESTART_TIME is defined
    if(sntpTimestamp == ESP32_RESTART_TIME) {
      esp32Restart(ESP32_RESTART_SYSTEM);
    }
  #endif
  
  // handle ESP32 Halt
  if(esp32Halt) {
    esp32Halt = false;
    
    Serial.println("ESP32 Halt...");
    #ifdef OTA_OUTPUT
      WebSerial.println("ESP32 Halt...");
    #endif
    
    delay(ESP32_HALT_DELAY);
    esp32Restart(ESP32_RESTART_OTA_HALT);
  }
  
  // handle WebSerial loop
  #ifdef OTA_OUTPUT
    WebSerial.loop();
  #endif
  
  // handle MQTT loop
  mqttClient.loop();
  delay(MQTT_LOOP_DELAY);
  
  // output wp states
  if(refreshOutputStateChrono.hasPassed(STATE_OUTPUT_REFRESH_INTERVAL_CHRONO)) {
    outputInfoline();
    refreshOutputStateChrono.restart();
  }
  
  // handle wp data
  if(wpDataHandleChrono.hasPassed(WP_DATA_HANDLE_INTERVAL_CHRONO) || (wpDataHandleCounter == 0)) {
    wpDataHandleCounter++;
    
    mqttTopicID = "WP";
    
    // MQTT server disconnect
    mqttClient.disconnect();
    delay(CONNECTION_DISCONNECT_DELAY);
    
    // WP server connect
    #ifdef DEBUG_OUTPUT
      sprintf(msgString, "WP server <%s> connecting...", wpServerIP);
      Serial.print(msgString);
    #endif
    #ifdef OTA_OUTPUT
      sprintf(msgString, "WP server <%s> connecting...", wpServerIP);
      WebSerial.print(msgString);
    #endif
    
    while(!wifiConnectionID.connect(wpServerIP, wpServerPort) && (wpReconnectionCounter < wpMaxReconnections)) {
      wpReconnectionCounter++;
      #ifdef DEBUG_OUTPUT
        Serial.print("-");
      #endif
      #ifdef OTA_OUTPUT
        WebSerial.print("-");
      #endif
      
      delay(CONNECTION_RETRY_DELAY);
    }
    if(wpReconnectionCounter >= wpMaxReconnections) {
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "\nWP TCP Error: WP server not connected after <%u> retries", wpReconnectionCounter);
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "\nWP TCP Error: WP server not connected after <%u> retries", wpReconnectionCounter);
        WebSerial.println(msgString);
      #endif
      
      esp32Restart(ESP32_RESTART_WP_TCP_CONNECTION);
    }
    #ifdef DEBUG_OUTPUT
      Serial.println("\nWP server connected.\n");
    #endif
    #ifdef OTA_OUTPUT
      WebSerial.print("\nWP server connected.\n");
    #endif
    
    // initalize wp communication (transmit tcp commands 'get parameter' + 'execute')
    wifiConnectionID.flush();
    tcpRxBuffer[tcpRxBufferSize] = {0};
    wpCommandTransmitted = 0;
    
    tcpTxCounter = tcpTransmitBuffer(wpTCPCommandGetParameterSequence);  // tcp transmit 'get parameter'
    if(tcpTxCounter == tcpTxPacketSize) {
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "WP TCP command 'get parameter' <0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x> transmitted", wpTCPCommandGetParameterSequence[0], wpTCPCommandGetParameterSequence[1], wpTCPCommandGetParameterSequence[2], wpTCPCommandGetParameterSequence[3]);
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "WP TCP command 'get parameter' <0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x> transmitted", wpTCPCommandGetParameterSequence[0], wpTCPCommandGetParameterSequence[1], wpTCPCommandGetParameterSequence[2], wpTCPCommandGetParameterSequence[3]);
        WebSerial.println(msgString);
      #endif
      delay(TCP_TX_DELAY);
    } else {
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "WP TCP Error: TCP command 'get parameter' not transmitted!\n");
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "WP TCP Error: TCP command 'get parameter' not transmitted!\n");
        WebSerial.println(msgString);
      #endif
      
      esp32Restart(ESP32_RESTART_WP_TCP_COMMUNICATION);
    }
    tcpTxCounter = tcpTransmitBuffer(wpTCPCommandExecuteSequence);  // tcp transmit 'execute'
    if(tcpTxCounter == tcpTxPacketSize) {
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "WP TCP command 'execute' <0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x> transmitted\n", wpTCPCommandExecuteSequence[0], wpTCPCommandExecuteSequence[1], wpTCPCommandExecuteSequence[2], wpTCPCommandExecuteSequence[3]);
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "WP TCP command 'execute' <0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x> transmitted\n", wpTCPCommandExecuteSequence[0], wpTCPCommandExecuteSequence[1], wpTCPCommandExecuteSequence[2], wpTCPCommandExecuteSequence[3]);
        WebSerial.println(msgString);
      #endif
      delay(TCP_TX_DELAY);
    } else {
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "WP TCP Error: TCP command 'execute' not transmitted!\n");
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "WP TCP Error: TCP command 'execute' not transmitted!\n");
        WebSerial.println(msgString);
      #endif
      
      esp32Restart(ESP32_RESTART_WP_TCP_COMMUNICATION);
    }
    delay(TCP_COMMUNICATION_DELAY);

    // read all bytes from tcp receive buffer
    tcpRxByteAvailable = wifiConnectionID.available();
    if(tcpRxByteAvailable) {
      tcpRxByteReceived = wifiConnectionID.read(tcpRxBuffer, tcpRxByteAvailable);  // tcp receive data
      delay(TCP_RX_DELAY);
      
      // check received bytes
      if(tcpRxByteReceived == wpRxRequestedByteNumber) {
        #ifdef DEBUG_OUTPUT
          sprintf(msgString, "WP TCP received <%u/%u> bytes", tcpRxByteReceived, wpRxRequestedByteNumber);
          Serial.println(msgString);
        #endif
        #ifdef OTA_OUTPUT
          sprintf(msgString, "WP TCP received <%u/%u> bytes", tcpRxByteReceived, wpRxRequestedByteNumber);
          WebSerial.println(msgString);
        #endif
      } else {
        #ifdef DEBUG_OUTPUT
          sprintf(msgString, "\nWP TCP Error: less than <%u/%u> bytes received!", tcpRxByteReceived, wpRxRequestedByteNumber);
          Serial.println(msgString);
        #endif
        #ifdef OTA_OUTPUT
          sprintf(msgString, "\nWP TCP Error: less than <%u/%u> bytes received!", tcpRxByteReceived, wpRxRequestedByteNumber);
          WebSerial.println(msgString);
        #endif
        
        esp32Restart(ESP32_RESTART_WP_TCP_COMMUNICATION);
      }  // end if tcpRxByteReceived
    } else {
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "\nWP TCP Error: no data received!");
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "\nWP TCP Error: no data received!");
        WebSerial.println(msgString);
      #endif
      
      esp32Restart(ESP32_RESTART_WP_TCP_COMMUNICATION);
    }  // end if tcpRxByteAvailable
    
    // check received tcp commands
    tcpCheckGetValue = ((tcpRxBuffer[0] << 24) + (tcpRxBuffer[1] << 16) + (tcpRxBuffer[2] << 8) + tcpRxBuffer[3]);  // bytes 0-3
    tcpCheckExecute = ((tcpRxBuffer[4] << 24) + (tcpRxBuffer[5] << 16) + (tcpRxBuffer[6] << 8) + tcpRxBuffer[7]);  // bytes 4-7
    tcpCheckParameters = ((tcpRxBuffer[8] << 24) + (tcpRxBuffer[9] << 16) + (tcpRxBuffer[10] << 8) + tcpRxBuffer[11]);  // bytes 8-11
    
    if((tcpCheckGetValue == wpTCPCommandGetParameter) && (tcpCheckExecute == 0)) {
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "WP TCP received valid commands <%lu | %lu | %lu>", tcpCheckGetValue, tcpCheckExecute, tcpCheckParameters);
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "WP TCP received valid commands <%lu | %lu | %lu>", tcpCheckGetValue, tcpCheckExecute, tcpCheckParameters);
        WebSerial.println(msgString);
      #endif
      
      wpCommandTransmitted = 1;
    } else {
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "\nWP TCP Error: invalid received check commands <%lu | %lu | %lu>!", tcpCheckGetValue, tcpCheckExecute, tcpCheckParameters);
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "\nWP TCP Error: invalid received check commands <%lu | %lu | %lu>!", tcpCheckGetValue, tcpCheckExecute, tcpCheckParameters);
        WebSerial.println(msgString);
      #endif
      
      esp32Restart(ESP32_RESTART_WP_TCP_COMMUNICATION);
    }
    #ifdef DEBUG_OUTPUT
      Serial.println();
    #endif
    #ifdef OTA_OUTPUT
      WebSerial.println();
    #endif
    
    // handle adjust wp system time
		if(WP_SYSTEM_TIME_ADJUST) {  // check wp system time adjustment
      wpRegisterBlockName = "AdjustSysTime";
      wpEntityRegisterAddress = wpRegisterBlockSysTime[systenTimeBlockIndex].wpRegisterAddress;
      tcpBufferIndex = (wpEntityRegisterAddress * tcpRxPacketSize);
      
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "%s/%s", String(mqttTopicID).c_str(), String(wpRegisterBlockName).c_str());
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "%s/%s", String(mqttTopicID).c_str(), String(wpRegisterBlockName).c_str());
        WebSerial.println(msgString);
      #endif
      
      if(sntpUnixTimestamp > BASE_UNIX_TIMESTAMP) {  // sntp unix timestamp validation
        // handle wp system time
        wpUnixTimestamp = ((tcpRxBuffer[tcpBufferIndex + 0] << 24) + (tcpRxBuffer[tcpBufferIndex + 1] << 16) + (tcpRxBuffer[tcpBufferIndex + 2] << 8) + tcpRxBuffer[tcpBufferIndex + 3]);
        wpUnixTime.getDateTime(wpUnixTimestamp + (timeZoneAdjustment * 3600));  // convert unix time -> date/time (yyyy-mm-dd hh:mm:ss)
        sprintf(wpDateTime, "%0.4d-%0.2d-%0.2d / %0.2d:%0.2d:%0.2d", wpUnixTime.year, wpUnixTime.month, wpUnixTime.day, wpUnixTime.hour, wpUnixTime.minute, wpUnixTime.second);
        
        wpUnixTimestampDiff = abs(int32_t(sntpUnixTimestamp - wpUnixTimestamp));
			  
        if(wpUnixTimestampDiff > NTP_WP_TIMESTAMP_DIFF) {  // wp system time != ntp time (>= NTP_WP_TIMESTAMP_DIFF)
          // convert ntp unix timestamp to wp parameter value
          wpTCPSetParameterValueSequence[0] = sntpUnixTimestamp >> 24;
          wpTCPSetParameterValueSequence[1] = sntpUnixTimestamp >> 16;
          wpTCPSetParameterValueSequence[2] = sntpUnixTimestamp >> 8;
          wpTCPSetParameterValueSequence[3] = sntpUnixTimestamp;
          
          // initalize wp communication (transmit tcp commands 'set parameter' + 'parameter system time' + 'parameter value'
          wifiConnectionID.flush();
          tcpRxBuffer[tcpRxBufferSize] = {0};
          
          tcpTxCounter = tcpTransmitBuffer(wpTCPCommandSetParameterSequence);  // tcp transmit 'set parameter'
          if(tcpTxCounter == tcpTxPacketSize) {
            #ifdef DEBUG_OUTPUT
              sprintf(msgString, "WP TCP command 'set parameter' <0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x> transmitted", wpTCPCommandSetParameterSequence[0], wpTCPCommandSetParameterSequence[1], wpTCPCommandSetParameterSequence[2], wpTCPCommandSetParameterSequence[3]);
              Serial.println(msgString);
            #endif
            #ifdef OTA_OUTPUT
              sprintf(msgString, "WP TCP command 'set parameter' <0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x> transmitted", wpTCPCommandSetParameterSequence[0], wpTCPCommandSetParameterSequence[1], wpTCPCommandSetParameterSequence[2], wpTCPCommandSetParameterSequence[3]);
              WebSerial.println(msgString);
            #endif
            delay(TCP_TX_DELAY);
          } else {
            #ifdef DEBUG_OUTPUT
              sprintf(msgString, "WP TCP Error: TCP command 'set parameter' not transmitted!\n");
              Serial.println(msgString);
            #endif
            #ifdef OTA_OUTPUT
              sprintf(msgString, "WP TCP Error: TCP command 'set parameter' not transmitted!\n");
              WebSerial.println(msgString);
            #endif
            
            esp32Restart(ESP32_RESTART_WP_TCP_COMMUNICATION);
          }
          tcpTxCounter = tcpTransmitBuffer(wpTCPSetParameterSystemTimeSequence);  // tcp transmit 'parameter system time'
          if(tcpTxCounter == tcpTxPacketSize) {
            #ifdef DEBUG_OUTPUT
              sprintf(msgString, "WP TCP command 'set parameter system time' <0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x> transmitted", wpTCPSetParameterSystemTimeSequence[0], wpTCPSetParameterSystemTimeSequence[1], wpTCPSetParameterSystemTimeSequence[2], wpTCPSetParameterSystemTimeSequence[3]);
              Serial.println(msgString);
            #endif
            #ifdef OTA_OUTPUT
              sprintf(msgString, "WP TCP command 'set parameter system time' <0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x> transmitted", wpTCPSetParameterSystemTimeSequence[0], wpTCPSetParameterSystemTimeSequence[1], wpTCPSetParameterSystemTimeSequence[2], wpTCPSetParameterSystemTimeSequence[3]);
              WebSerial.println(msgString);
            #endif
            delay(TCP_TX_DELAY);
          } else {
            #ifdef DEBUG_OUTPUT
              sprintf(msgString, "WP TCP Error: TCP command 'set parameter system time' not transmitted!\n");
              Serial.println(msgString);
            #endif
            #ifdef OTA_OUTPUT
              sprintf(msgString, "WP TCP Error: TCP command 'set parameter system time' not transmitted!\n");
              WebSerial.println(msgString);
            #endif
            
            esp32Restart(ESP32_RESTART_WP_TCP_COMMUNICATION);
          }
          tcpTxCounter = tcpTransmitBuffer(wpTCPSetParameterValueSequence);  // tcp transmit 'parameter value' <unix timestamp>
          if(tcpTxCounter == tcpTxPacketSize) {
            #ifdef DEBUG_OUTPUT
              sprintf(msgString, "WP TCP command 'set parameter value' <0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x> transmitted\n", wpTCPSetParameterValueSequence[0], wpTCPSetParameterValueSequence[1], wpTCPSetParameterValueSequence[2], wpTCPSetParameterValueSequence[3]);
              Serial.println(msgString);
            #endif
            #ifdef OTA_OUTPUT
              sprintf(msgString, "WP TCP command 'set parameter value' <0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x> transmitted\n", wpTCPSetParameterValueSequence[0], wpTCPSetParameterValueSequence[1], wpTCPSetParameterValueSequence[2], wpTCPSetParameterValueSequence[3]);
              WebSerial.println(msgString);
            #endif
          delay(TCP_TX_DELAY);
          } else {
            #ifdef DEBUG_OUTPUT
              sprintf(msgString, "WP TCP Error: TCP command 'set parameter value' not transmitted!\n");
              Serial.println(msgString);
            #endif
            #ifdef OTA_OUTPUT
              sprintf(msgString, "WP TCP Error: TCP TCP command 'set parameter value' not transmitted!\n");
              WebSerial.println(msgString);
            #endif
            
            esp32Restart(ESP32_RESTART_WP_TCP_COMMUNICATION);
          }
          delay(TCP_COMMUNICATION_DELAY);
          
          // read all bytes from tcp receive buffer
          tcpRxByteAvailable = wifiConnectionID.available();
          if(tcpRxByteAvailable) {
            tcpRxByteReceived = wifiConnectionID.read(tcpRxBuffer, tcpRxByteAvailable);  // tcp receive data
            delay(TCP_RX_DELAY);
            
            sprintf(msgString, "WP TCP received <%lu> bytes", tcpRxByteReceived);
            Serial.println(msgString);
            sprintf(msgString, "bytes[%lu]: <0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x> <0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x>\n", tcpRxByteReceived, tcpRxBuffer[0], tcpRxBuffer[1], tcpRxBuffer[2], tcpRxBuffer[3], tcpRxBuffer[4], tcpRxBuffer[5], tcpRxBuffer[6], tcpRxBuffer[7]);
            Serial.println(msgString);
          } else {
            sprintf(msgString, "\nWP TCP Error: no data received!");
            Serial.println(msgString);
          }
          
          sprintf(msgString, "sntpUnixTimestamp <%lu> / wpUnixTimestamp <%lu> / wpUnixTimestampDiff <%lu>", sntpUnixTimestamp, wpUnixTimestamp, wpUnixTimestampDiff);
          Serial.println(msgString);
          
          sprintf(msgString, "SNTP Date/Time: <%s>", String(sntpDateTime).c_str());
          Serial.println(msgString);
          
          sprintf(msgString, "WP System Date/Time: <%s>", String(wpDateTime).c_str());
          Serial.println(msgString);
          
          sprintf(msgString, "WP System Date/Time adjust: <%s> / <%lu> / <0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x>\n", String(sntpDateTime).c_str(), sntpUnixTimestamp, wpTCPSetParameterValueSequence[0], wpTCPSetParameterValueSequence[1], wpTCPSetParameterValueSequence[2], wpTCPSetParameterValueSequence[3]);
          Serial.println(msgString);
        } else {
          sprintf(msgString, "sntpUnixTimestamp <%lu> / wpUnixTimestamp <%lu> / wpUnixTimestampDiff <%lu>", sntpUnixTimestamp, wpUnixTimestamp, wpUnixTimestampDiff);
          Serial.println(msgString);
          
          sprintf(msgString, "SNTP Date/Time: <%s>", String(sntpDateTime).c_str());
          Serial.println(msgString);
          sprintf(msgString, "WP System Date/Time adjust: <%s>", String(wpDateTime).c_str());
          Serial.println(msgString);          
          sprintf(msgString, "WP System Time OK...\n", sntpUnixTimestamp);
          Serial.println(msgString);
        }
      } else {
        sprintf(msgString, "SNTP Unix Timestamp <%lu> not valid!\n", sntpUnixTimestamp);
        Serial.println(msgString);
      }  // sntp unix timestamp validation
		}  // check wp system time adjustment
    
    // WP server disconnect
    wifiConnectionID.stop();
    delay(TCP_DISCONNECT_DELAY);
    
    // MQTT server connect
    #ifdef DEBUG_OUTPUT
      sprintf(msgString, "MQTT server <%s> connecting...", mqttServer);
      Serial.print(msgString);
    #endif
    #ifdef OTA_OUTPUT
      sprintf(msgString, "MQTT server <%s> connecting...", mqttServer);
      WebSerial.print(msgString);
    #endif
    
    mqttClient.begin(mqttServer, mqttServerPort, wifiConnectionID);
    delay(CONNECTION_DELAY);
    mqttReconnectionCounter = 0;
    while(!mqttClient.connected() && (mqttReconnectionCounter < mqttMaxReconnections)) {
      mqttReconnectionCounter++;
      mqttClient.connect(mqttClientID, mqttUser, mqttPassword, mqttDisconnectSkip);
      #ifdef DEBUG_OUTPUT
        Serial.print("-");
      #endif
      #ifdef OTA_OUTPUT
        WebSerial.print("-");
      #endif
      delay(CONNECTION_RETRY_DELAY);
    }
    if(mqttReconnectionCounter >= mqttMaxReconnections) {
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "\nMQTT Error: MQTT server not connected after <%u> retries", mqttReconnectionCounter);
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "\nMQTT Error: MQTT server not connected after <%u> retries", mqttReconnectionCounter);
        WebSerial.println(msgString);
      #endif
      
      esp32Restart(ESP32_RESTART_MQTT_CONNECTION);
    }
    #ifdef DEBUG_OUTPUT
      Serial.println("\nMQTT server connected.\n");
    #endif
    #ifdef OTA_OUTPUT
      WebSerial.print("\nMQTT server connected.\n");
    #endif
    
    if(wpRegisterBlockIndex == 0) {  // only once per wp register block cycle
      // handle wp system time
      wpRegisterBlockName = "SysTime";
      wpEntityRegisterAddress = wpRegisterBlockSysTime[systenTimeBlockIndex].wpRegisterAddress;
      tcpBufferIndex = (wpEntityRegisterAddress * tcpRxPacketSize);
      
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "%s/%s", String(mqttTopicID).c_str(), String(wpRegisterBlockName).c_str());
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "%s/%s", String(mqttTopicID).c_str(), String(wpRegisterBlockName).c_str());
        WebSerial.println(msgString);
      #endif
      
      // handle wp system time
      wpUnixTimestamp = ((tcpRxBuffer[tcpBufferIndex + 0] << 24) + (tcpRxBuffer[tcpBufferIndex + 1] << 16) + (tcpRxBuffer[tcpBufferIndex + 2] << 8) + tcpRxBuffer[tcpBufferIndex + 3]);
      wpUnixTime.getDateTime(wpUnixTimestamp + (timeZoneAdjustment * 3600));  // convert unix time -> date/time (yyyy-mm-dd hh:mm:ss)
      sprintf(wpDateTime, "%0.4d-%0.2d-%0.2d / %0.2d:%0.2d:%0.2d", wpUnixTime.year, wpUnixTime.month, wpUnixTime.day, wpUnixTime.hour, wpUnixTime.minute, wpUnixTime.second);
      
      mqttTopicEntity = "System_Time";
      mqttTopic = String(mqttTopicID + "/" + mqttTopicEntity).c_str();
      mqttPayload = ("{\"" + String(mqttTopicEntity) + "\":\"" + String(wpDateTime) + "\"}").c_str();
      mqttSuccess = mqttClient.publish(mqttTopic, mqttPayload, mqttRetained, mqttQoS);
      
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "%s: ", String(mqttTopicEntity).c_str());
        Serial.print(msgString);
        Serial.println(String(wpDateTime).c_str());
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "%s: ", String(mqttTopicEntity).c_str());
        WebSerial.print(msgString);
        WebSerial.println(String(wpDateTime).c_str());
      #endif
    }
    
    // handle wp data
    switch(wpRegisterBlockIndex) {
      case 0: {  // SysValues
        wpRegisterBlockName = "SysValues";
        if(wpRegisterBlocks[wpRegisterBlockIndex].wpRegisterBlockActive) {
          wpRegisterBlockHeadElement = wpRegisterBlocks[wpRegisterBlockIndex].wpRegisterBlockHeadElement;
          wpRegisterBlockElements = wpRegisterBlocks[wpRegisterBlockIndex].wpRegisterBlockElements;
          
          // handle tcp holding register data
          #ifdef DEBUG_OUTPUT
            sprintf(msgString, "%s/%s", String(mqttTopicID).c_str(), String(wpRegisterBlockName).c_str());
            Serial.println(msgString);
          #endif
          #ifdef OTA_OUTPUT
            sprintf(msgString, "%s/%s", String(mqttTopicID).c_str(), String(wpRegisterBlockName).c_str());
            WebSerial.println(msgString);
          #endif
          
          for(unsigned short element = 0; element < wpRegisterBlockElements; element++) {
            wpEntityRegisterAddress = wpRegisterSysValues[element].wpRegisterAddress;
            wpEntityDescriptor = wpRegisterSysValues[element].wpRegisterDescriptor;
            wpEntityDataType = wpRegisterSysValues[element].wpRegisterDataType;
            wpEntityDataFactor = wpRegisterSysValues[element].wpRegisterDataFactor;
            wpEntityUnit = wpRegisterSysValues[element].wpRegisterUnit;
            wpEntityActiveMQTT = wpRegisterSysValues[element].wpRegisterActiveMQTT;
            
            tcpBufferIndex = (wpEntityRegisterAddress * tcpRxPacketSize);  // (wpEntity...Address * 4), ((wpEntity...Address + 1) * 4), ((wpEntity...Address + 1) * 4), ..., (wpRegisterBlockElements * 4)
            
            if(wpEntityRegisterAddress == WP_REGISTER_VALUES_STATE_WP) {  // special case value wp state
              uint32_t wpValueU32 = ((tcpRxBuffer[tcpBufferIndex + 0] << 24) + (tcpRxBuffer[tcpBufferIndex + 1] << 16) + (tcpRxBuffer[tcpBufferIndex + 2] << 8) + tcpRxBuffer[tcpBufferIndex + 3]);
              
              wpStateID = wpValueU32;  // global wp state ID
              wpState = wpRegisterSysValuesState[wpStateID].wpRegisterStateDescriptor;  // global wp state
            }
            
            if(wpEntityRegisterAddress == WP_REGISTER_VALUES_SHUTDOWN_REASON0) {  // special cases value shutdown reason
              uint32_t wpValueU32 = ((tcpRxBuffer[tcpBufferIndex + 0] << 24) + (tcpRxBuffer[tcpBufferIndex + 1] << 16) + (tcpRxBuffer[tcpBufferIndex + 2] << 8) + tcpRxBuffer[tcpBufferIndex + 3]);
              
              wpShutdownReasonID = (wpValueU32 - 1);
              wpShutdownReason[0] = wpRegisterSysValuesShutdownReason[wpShutdownReasonID].wpRegisterShutdownReasonDescriptor;  // global wp shutdown reason 0
            } else if(wpEntityRegisterAddress == WP_REGISTER_VALUES_SHUTDOWN_REASON1) {
              uint32_t wpValueU32 = ((tcpRxBuffer[tcpBufferIndex + 0] << 24) + (tcpRxBuffer[tcpBufferIndex + 1] << 16) + (tcpRxBuffer[tcpBufferIndex + 2] << 8) + tcpRxBuffer[tcpBufferIndex + 3]);
              
              wpShutdownReasonID = (wpValueU32 - 1);
              wpShutdownReason[1] = wpRegisterSysValuesShutdownReason[wpShutdownReasonID].wpRegisterShutdownReasonDescriptor;  // global wp shutdown reason 0
            } else if(wpEntityRegisterAddress == WP_REGISTER_VALUES_SHUTDOWN_REASON2) {
              uint32_t wpValueU32 = ((tcpRxBuffer[tcpBufferIndex + 0] << 24) + (tcpRxBuffer[tcpBufferIndex + 1] << 16) + (tcpRxBuffer[tcpBufferIndex + 2] << 8) + tcpRxBuffer[tcpBufferIndex + 3]);
              
              wpShutdownReasonID = (wpValueU32 - 1);
              wpShutdownReason[2] = wpRegisterSysValuesShutdownReason[wpShutdownReasonID].wpRegisterShutdownReasonDescriptor;  // global wp shutdown reason 0
            } else if(wpEntityRegisterAddress == WP_REGISTER_VALUES_SHUTDOWN_REASON3) {
              uint32_t wpValueU32 = ((tcpRxBuffer[tcpBufferIndex + 0] << 24) + (tcpRxBuffer[tcpBufferIndex + 1] << 16) + (tcpRxBuffer[tcpBufferIndex + 2] << 8) + tcpRxBuffer[tcpBufferIndex + 3]);
              
              wpShutdownReasonID = (wpValueU32 - 1);
              wpShutdownReason[3] = wpRegisterSysValuesShutdownReason[wpShutdownReasonID].wpRegisterShutdownReasonDescriptor;  // global wp shutdown reason 0
            } else if(wpEntityRegisterAddress == WP_REGISTER_VALUES_SHUTDOWN_REASON4) {
              uint32_t wpValueU32 = ((tcpRxBuffer[tcpBufferIndex + 0] << 24) + (tcpRxBuffer[tcpBufferIndex + 1] << 16) + (tcpRxBuffer[tcpBufferIndex + 2] << 8) + tcpRxBuffer[tcpBufferIndex + 3]);
              
              wpShutdownReasonID = (wpValueU32 - 1);
              wpShutdownReason[4] = wpRegisterSysValuesShutdownReason[wpShutdownReasonID].wpRegisterShutdownReasonDescriptor;  // global wp shutdown reason 0
            }
            
            outputWPData(tcpRxBuffer, tcpBufferIndex, wpEntityDescriptor, wpEntityDataType, wpEntityDataFactor, wpEntityUnit, wpEntityActiveMQTT, mqttRetained, mqttQoS);
            
            if((element % WEB_SERIAL_MAX_OUTPUT_COUNT) == 0) {  // check web serial output sequence count
              delay(WEB_SERIAL_MAX_OUTPUT_DELAY);
            }
          }
          wpDataHandleChrono.restart();  // restart chrono
          
          #ifdef DEBUG_OUTPUT
            Serial.println();
          #endif
          #ifdef OTA_OUTPUT
            WebSerial.println();
          #endif
        }  // end if wpRegisterBlockActive
        wpRegisterBlockIndex = 0;  // only one wp register block
        break;
      }  // end case
      default: {
        wpRegisterBlockIndex = 0;  // reset wp register block index
        break;
      }  // end default
    }  // end switch wpRegisterBlockIndex
  }  // end if wpDataHandleChrono.hasPassed
}  // end loop()

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void esp32Restart(uint8_t esp32RestartID) {
  String mqttTopicID = "ESP32";
  String mqttTopicEntity = "Restart";
  String mqttPayload="1";
  bool mqttSuccess;
  
  // adjust+store NVS ESP32 restart counters
  esp32RestartInfo[esp32RestartID].esp32RestartCounter++;
  nvs.putUInt(esp32RestartInfo[esp32RestartID].esp32RestartReasonNVS, esp32RestartInfo[esp32RestartID].esp32RestartCounter);
  
  sprintf(msgString, "ESP32 Restarting [%s]...", esp32RestartInfo[esp32RestartID].esp32RestartReason);
  Serial.println(msgString);
  
  #ifdef OTA_OUTPUT
    sprintf(msgString, "ESP32 Restarting [%s]...", esp32RestartInfo[esp32RestartID].esp32RestartReason);
    WebSerial.println(msgString);
  #endif
  
  // MQTT publish
  mqttTopic = String(mqttTopicID + "/" + mqttTopicEntity).c_str();
  mqttPayload = ("{\"" + String(mqttTopicEntity) + "\":\"" + String(mqttPayload) + "\"}").c_str();
  mqttSuccess = mqttClient.publish(mqttTopic, mqttPayload, mqttRetained, mqttQoS);
  delay(MQTT_PUBLISH_DELAY);
  
  // reset connections
  mqttClient.disconnect();
  delay(CONNECTION_DELAY);
  esp_sntp_stop();
  delay(CONNECTION_DELAY);
  wifiConnectionID.stop();
  delay(CONNECTION_DELAY);
  WiFi.disconnect();
  
  // NVS end
  nvs.end();
  
  delay(ESP32_RESTART_DELAY);
  ESP.restart();
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void serialOTAReceiver(uint8_t *data, size_t length) {
  String buffer = "";
  
  for(uint8_t i = 0; i < length; i++) {
    buffer += char(data[i]);
  }
  
  if(buffer == "Restart") {
    esp32Restart(ESP32_RESTART_OTA_RESTART);
  } else if(buffer == "Halt") {
    esp32Halt = 1;
  } else if(buffer == "Clear") {
    // clear NVS
    nvs.clear();
    
    // read NVS values
    for(uint8_t i = 0; i < ESP32_RESTART_INFO_ELEMENTS; i++) {
      esp32RestartInfo[i].esp32RestartCounter = nvs.getUInt(esp32RestartInfo[i].esp32RestartReasonNVS);
    }
    
    WebSerial.println("ESP32: NVS cleared.");
  } else {
    WebSerial.println("Usage: \'Restart\' | \'Halt\' | \'Clear\'");
  }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
unsigned int tcpTransmitBuffer(uint8_t *tcpTxBuffer) {
  unsigned int tcpTxBytes = 0;

  tcpTxBytes = wifiConnectionID.write(tcpTxBuffer, sizeof(tcpTxBuffer));
  
  return(tcpTxBytes);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void outputInfoline() {
  #ifdef DEBUG_OUTPUT
    sprintf(msgString, "WP CTA Aeroheat CI 8is Prod [%s] - %s (%s)", String(wpState).c_str(), String(wpDateTime).c_str(), sntpTimeZone);
    Serial.println(msgString);
    
    sprintf(msgString, "ESP32 Restart Counters: [%lu | %lu | %lu | %lu | %lu | %lu | %lu] - [%s | %s | %s | %s | %s | %s | %s]", \
                                                 esp32RestartInfo[0].esp32RestartCounter, esp32RestartInfo[1].esp32RestartCounter, esp32RestartInfo[2].esp32RestartCounter, esp32RestartInfo[3].esp32RestartCounter, esp32RestartInfo[4].esp32RestartCounter, esp32RestartInfo[5].esp32RestartCounter, esp32RestartInfo[6].esp32RestartCounter, \
                                                 esp32RestartInfo[0].esp32RestartReasonNVS, esp32RestartInfo[1].esp32RestartReasonNVS, esp32RestartInfo[2].esp32RestartReasonNVS, esp32RestartInfo[3].esp32RestartReasonNVS, esp32RestartInfo[4].esp32RestartReasonNVS, esp32RestartInfo[5].esp32RestartReasonNVS, esp32RestartInfo[6].esp32RestartReasonNVS);
    Serial.println(msgString);
    
    Serial.println();
  #endif
  
  #ifdef OTA_OUTPUT
    sprintf(msgString, "WP CTA Aeroheat CI 8is Prod [%s] - %s (%s)", String(wpState).c_str(), String(wpDateTime).c_str(), sntpTimeZone);
    WebSerial.println(msgString);
    
    sprintf(msgString, "ESP32 Restart Counters: [%lu | %lu | %lu | %lu | %lu | %lu | %lu] - [%s | %s | %s | %s | %s | %s | %s]", \
                                                 esp32RestartInfo[0].esp32RestartCounter, esp32RestartInfo[1].esp32RestartCounter, esp32RestartInfo[2].esp32RestartCounter, esp32RestartInfo[3].esp32RestartCounter, esp32RestartInfo[4].esp32RestartCounter, esp32RestartInfo[5].esp32RestartCounter, esp32RestartInfo[6].esp32RestartCounter, \
                                                 esp32RestartInfo[0].esp32RestartReasonNVS, esp32RestartInfo[1].esp32RestartReasonNVS, esp32RestartInfo[2].esp32RestartReasonNVS, esp32RestartInfo[3].esp32RestartReasonNVS, esp32RestartInfo[4].esp32RestartReasonNVS, esp32RestartInfo[5].esp32RestartReasonNVS, esp32RestartInfo[6].esp32RestartReasonNVS);
    WebSerial.println(msgString);
    
    WebSerial.println();
  #endif
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void outputWPData(uint8_t *tcpBuffer, uint16_t tcpBufferIndex, char *wpEntityDescriptor, char *wpEntityDataType, float wpEntityDataFactor, char *wpEntityUnit, bool mqttActive, bool mqttRetained, uint16_t mqttQoS) {
  String mqttTopicID = "WP";
  UnixTime unixTime(GMT);
  bool mqttSuccess;
  
  if((wpEntityDataType == "U32") || (wpEntityDataType == "Status") || (wpEntityDataType == "I/O")) {
    uint32_t bufferValueUInt32 = ((tcpBuffer[tcpBufferIndex + 0] << 24) + (tcpBuffer[tcpBufferIndex + 1] << 16) + (tcpBuffer[tcpBufferIndex + 2] << 8) + tcpBuffer[tcpBufferIndex + 3]);
    
    if(wpEntityDescriptor == "Abschalt_Ursache0") {  
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "%s: %s[%u]", String(wpEntityDescriptor).c_str(), String(wpShutdownReason[0]).c_str(), bufferValueUInt32);
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "%s: %s[%u]", String(wpEntityDescriptor).c_str(), String(wpShutdownReason[0]).c_str(), bufferValueUInt32);
        WebSerial.println(msgString);
      #endif
      
      sprintf(msgString, "%s[%u]", String(wpShutdownReason[0]).c_str(), bufferValueUInt32);
      if(mqttActive) {
        mqttSuccess = mqttPublishTopicPayloadString(mqttTopicID, wpEntityDescriptor, msgString, mqttRetained, mqttQoS);
      }
    } else if(wpEntityDescriptor == "Abschalt_Ursache1") {  
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "%s: %s[%u]", String(wpEntityDescriptor).c_str(), String(wpShutdownReason[1]).c_str(), bufferValueUInt32);
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "%s: %s[%u]", String(wpEntityDescriptor).c_str(), String(wpShutdownReason[1]).c_str(), bufferValueUInt32);
        WebSerial.println(msgString);
      #endif
    } else if(wpEntityDescriptor == "Abschalt_Ursache2") {  
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "%s: %s[%u]", String(wpEntityDescriptor).c_str(), String(wpShutdownReason[2]).c_str(), bufferValueUInt32);
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "%s: %s[%u]", String(wpEntityDescriptor).c_str(), String(wpShutdownReason[2]).c_str(), bufferValueUInt32);
        WebSerial.println(msgString);
      #endif
    } else if(wpEntityDescriptor == "Abschalt_Ursache3") {  
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "%s: %s[%u]", String(wpEntityDescriptor).c_str(), String(wpShutdownReason[3]).c_str(), bufferValueUInt32);
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "%s: %s[%u]", String(wpEntityDescriptor).c_str(), String(wpShutdownReason[3]).c_str(), bufferValueUInt32);
        WebSerial.println(msgString);
      #endif
    } else if(wpEntityDescriptor == "Abschalt_Ursache4") {  
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "%s: %s[%u]", String(wpEntityDescriptor).c_str(), String(wpShutdownReason[4]).c_str(), bufferValueUInt32);
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "%s: %s[%u]", String(wpEntityDescriptor).c_str(), String(wpShutdownReason[4]).c_str(), bufferValueUInt32);
        WebSerial.println(msgString);
      #endif
    } else {
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "%s: %u%s", String(wpEntityDescriptor).c_str(), bufferValueUInt32, String(wpEntityUnit).c_str());
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "%s: %u%s", String(wpEntityDescriptor).c_str(), bufferValueUInt32, String(wpEntityUnit).c_str());
        WebSerial.println(msgString);
      #endif
    }
    
    if(mqttActive) {
      mqttSuccess = mqttPublishTopicPayloadUInt32(mqttTopicID, wpEntityDescriptor, bufferValueUInt32, mqttRetained, mqttQoS);
    }
  } else if(wpEntityDataType == "F32") {
    // float convertion method #1
    float bufferValueFloat32 = (((tcpBuffer[tcpBufferIndex + 0] << 24) + (tcpBuffer[tcpBufferIndex + 1] << 16) + (tcpBuffer[tcpBufferIndex + 2] << 8) + tcpBuffer[tcpBufferIndex + 3]) * wpEntityDataFactor);
    
    #ifdef DEBUG_OUTPUT
      sprintf(msgString, "%s: %0.2f%s", String(wpEntityDescriptor).c_str(), bufferValueFloat32, String(wpEntityUnit).c_str());
      Serial.println(msgString);
    #endif
    #ifdef OTA_OUTPUT
      sprintf(msgString, "%s: %0.2f%s", String(wpEntityDescriptor).c_str(), bufferValueFloat32, String(wpEntityUnit).c_str());
      WebSerial.println(msgString);
    #endif
    
    if(mqttActive) {
      mqttSuccess = mqttPublishTopicPayloadFloat32(mqttTopicID, wpEntityDescriptor, bufferValueFloat32, mqttRetained, mqttQoS);
    }
  } else if(wpEntityDataType == "UX") {
    uint32_t bufferValueUInt32 = ((tcpBuffer[tcpBufferIndex + 0] << 24) + (tcpBuffer[tcpBufferIndex + 1] << 16) + (tcpBuffer[tcpBufferIndex + 2] << 8) + tcpBuffer[tcpBufferIndex + 3]);
    
    // convert unix timestamp
    unixTime.getDateTime(bufferValueUInt32 + (timeZoneAdjustment * 3600));  // convert unix time -> date/time (yyyy-mm-dd hh:mm:ss)
    
    #ifdef DEBUG_OUTPUT
      sprintf(msgString, "%s: %0.4d-%0.2d-%0.2d / %0.2d:%0.2d:%0.2d", String(wpEntityDescriptor).c_str(), unixTime.year, unixTime.month, unixTime.day, unixTime.hour, unixTime.minute, unixTime.second);
      Serial.println(msgString);
    #endif
    #ifdef OTA_OUTPUT
      sprintf(msgString, "%s: %0.4d-%0.2d-%0.2d / %0.2d:%0.2d:%0.2d", String(wpEntityDescriptor).c_str(), unixTime.year, unixTime.month, unixTime.day, unixTime.hour, unixTime.minute, unixTime.second);
      WebSerial.println(msgString);
    #endif
    
    if(mqttActive) {
      sprintf(msgString, "%0.4d-%0.2d-%0.2d / %0.2d:%0.2d:%0.2d", unixTime.year, unixTime.month, unixTime.day, unixTime.hour, unixTime.minute, unixTime.second);
      mqttSuccess = mqttPublishTopicPayloadString(mqttTopicID, wpEntityDescriptor, msgString, mqttRetained, mqttQoS);
    }
  } else if(wpEntityDataType == "IP") {
    #ifdef DEBUG_OUTPUT
      sprintf(msgString, "%s: %u.%u.%u.%u", String(wpEntityDescriptor).c_str(), tcpBuffer[tcpBufferIndex + 0], tcpBuffer[tcpBufferIndex + 1], tcpBuffer[tcpBufferIndex + 2], tcpBuffer[tcpBufferIndex + 3]);
      Serial.println(msgString);
    #endif
    #ifdef OTA_OUTPUT
      sprintf(msgString, "%s: %u.%u.%u.%u", String(wpEntityDescriptor).c_str(), tcpBuffer[tcpBufferIndex + 0], tcpBuffer[tcpBufferIndex + 1], tcpBuffer[tcpBufferIndex + 2], tcpBuffer[tcpBufferIndex + 3]);
      WebSerial.println(msgString);
    #endif
    
    if(mqttActive) {
      sprintf(msgString, "%u.%u.%u.%u", tcpBuffer[tcpBufferIndex + 0], tcpBuffer[tcpBufferIndex + 1], tcpBuffer[tcpBufferIndex + 2], tcpBuffer[tcpBufferIndex + 3]);
      mqttSuccess = mqttPublishTopicPayloadString(mqttTopicID, wpEntityDescriptor, msgString, mqttRetained, mqttQoS);
    }
  } else {
    #ifdef DEBUG_OUTPUT
      Serial.print("Unknown Type: ");
      Serial.println(wpEntityDataType);
    #endif
    #ifdef OTA_OUTPUT
      WebSerial.print("Unknown Type: ");
      WebSerial.println(wpEntityDataType);
    #endif
  }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
bool mqttPublishTopicPayloadString(String mqttTopicID, String mqttTopicEntity, String mqttPayloadString, bool mqttRetained, uint16_t mqttQoS) {
  String mqttTopic;
  String mqttPayload;
  bool mqttSuccess;
  
  mqttTopic = String(mqttTopicID + "/" + mqttTopicEntity).c_str();
  mqttPayload = ("{\"" + String(mqttTopicEntity) + "\":\"" + String(mqttPayloadString) + "\"}").c_str();
  mqttSuccess = mqttClient.publish(mqttTopic, mqttPayload, mqttRetained, mqttQoS);
  delay(MQTT_PUBLISH_DELAY);
  return(mqttSuccess);
}

bool mqttPublishTopicPayloadUInt32(String mqttTopicID, String mqttTopicEntity, uint32_t mqttPayloadUInt32, bool mqttRetained, uint16_t mqttQoS) {
  String mqttTopic;
  String mqttPayload;
  bool mqttSuccess;
  
  mqttTopic = String(mqttTopicID + "/" + mqttTopicEntity).c_str();
  mqttPayload = ("{\"" + String(mqttTopicEntity) + "\":\"" + String(mqttPayloadUInt32) + "\"}").c_str();
  mqttSuccess = mqttClient.publish(mqttTopic, mqttPayload, mqttRetained, mqttQoS);
  delay(MQTT_PUBLISH_DELAY);
  return(mqttSuccess);
}

bool mqttPublishTopicPayloadFloat32(String mqttTopicID, String mqttTopicEntity, float mqttPayloadFloat32, bool mqttRetained, uint16_t mqttQoS) {
  String mqttTopic;
  String mqttPayload;
  bool mqttSuccess;

  mqttTopic = String(mqttTopicID + "/" + mqttTopicEntity).c_str();
  mqttPayload = ("{\"" + String(mqttTopicEntity) + "\":\"" + String(mqttPayloadFloat32, 2) + "\"}").c_str();
  mqttSuccess = mqttClient.publish(mqttTopic, mqttPayload, mqttRetained, mqttQoS);
  delay(MQTT_PUBLISH_DELAY);
  return(mqttSuccess);
}

//EOF