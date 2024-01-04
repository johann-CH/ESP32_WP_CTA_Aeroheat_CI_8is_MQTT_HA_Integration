/*
  ESP32: WP CTA Aeroheat CI 8is Prod Home Assistant Integration (MQTT)
*/

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <Arduino.h>
#include <ESP32.h>

#include <Chrono.h>
#include <WiFi.h>
#include <UnixTime.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerialPro.h>

#include <MQTT.h>

#include "ESP32_WP_CTA_Aeroheat_CI_8is_MQTT.h"

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#define SERIAL_BAUDRATE 115200  // default serial interface bauderate
#define SERIAL_DELAY 200        // serial communication delay [ms]

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

#define WP_DATA_HANDLE_INTERVAL_CHRONO 10  // chrono wp register data handle interval [s]

#define STATE_OUTPUT_REFRESH_INTERVAL_CHRONO 10  // chrono wp + converter states output refresh interval [s]

#define WP_STATE_OFFLINE_CHANGE_LIMIT 5  // wp state "Offline" change limit
#define WP_STATE_OFFLINE_STAGE_LIMIT 10  // wp state "Offline" stage limit

#define DEBUG_OUTPUT  // default serial interface output active

#define OTA_OUTPUT  // over the air serial monitor output active

#define ESP32_RESTART_TIMESTAMP "06:00"  // ESP32 restart timestamp 06h00 (no restart if commented out)
#define ESP32_RESTART_LOCK_DELAY 60000   // ESP32 restart lock delay 60s [ms]
#define ESP32_RESTART_DELAY 5000         // ESP32 restart command delay [ms]

#define GMT_1 1  // Greenwich Mean Time +1

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// instantiate chrono objects
Chrono wpDataHandleChrono(Chrono::SECONDS);
Chrono refreshOutputStateChrono(Chrono::SECONDS);

// instantiate WiFi+MQTT objects
WiFiClient wifiConnectionID;
MQTTClient mqttClient(MQTT_IO_BUFFER_SIZE);

// instantiate over the air serial monitor
#ifdef OTA_OUTPUT
  AsyncWebServer serialOTA(80);
#endif

// instantiate unix time object
UnixTime unixTimestamp(GMT_1);

// WiFi definitions
const char wifiSSID[] = "WLAN_Belp";  // WiFi network SSID
const char wifiPassword[] = "family.ruch@belp";  // WiFi network password

unsigned int wifiReconnectionCounter = 0;
const unsigned int wifiMaxReconnections = 10;

// CTA Aeroheat CI 8is Prod definitions
const char wpServerIP[] = "192.168.1.190";
int wpServerPort = 8889;
const char wpUser[] = "";
const char wpPassword[] = "666666";
const char wpClientID[] = "wp";

// MQTT definitions
const char mqttServer[] = "192.168.1.150";
int mqttServerPort = 1883;
const char mqttUser[] = "mqtt-admin";
const char mqttPassword[] = "mqtt-admin";
const char mqttClientID[] = "wp_mqtt";
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
char *wpEntityDesriptor;
char *wpEntityDataType;
float wpEntityDataFactor;
char *wpEntityUnit;
bool wpEntityActiveMQTT;
uint16_t wpStateID = WP_STATE_OFFLINE;
char *wpState = "Offline";

// communication definitions
unsigned int wpReconnectionCounter = 0;
const unsigned int wpMaxReconnections = 10;

unsigned int mqttReconnectionCounter = 0;
const unsigned int mqttMaxReconnections = 10;

const unsigned int tcpRxPacketSize = 4;  // tcp receive packet size
const unsigned int tcpRxBufferSize = 2048;
uint8_t tcpRxBuffer[tcpRxBufferSize];  // wp tcp receive buffer
const unsigned int tcpTxPacketSize = 4;  // tcp transmit packet size
const unsigned int tcpTxBufferSize = 4;

unsigned int wpRxRequestedByteNumber = WP_REGISTER_VALUES_NUMBER_BYTES;
unsigned int tcpRxByteAvailable;
unsigned int tcpRxByteReceived;

uint8_t wpCommandGetValue[tcpTxBufferSize] = {0x00, 0x00, 0x0b, 0x0bc};  // '3004'
uint8_t wpCommandExecute[tcpTxBufferSize] = {0x00, 0x00, 0x00, 0x00};    // '0000'
unsigned long tcpCheckGetValue = ((tcpRxBuffer[0] << 24) + (tcpRxBuffer[1] << 16) + (tcpRxBuffer[2] << 8) + tcpRxBuffer[3]);  // bytes 0-3
unsigned long tcpCheckExecute = ((tcpRxBuffer[4] << 24) + (tcpRxBuffer[5] << 16) + (tcpRxBuffer[6] << 8) + tcpRxBuffer[7]);  // bytes 4-7

bool connectionsEstablished = 0;
bool wpCommandTransmitted = 0;

bool mqttSuccess;

// common definitions
char msgString[256];  // message char array

String wpRegisterBlockName;
unsigned int wpRegisterBlockIndex = 0;
unsigned int systenTimeBlockIndex = 0;
unsigned int tcpBufferIndex;

uint32_t wpTimestamp;
String wpTime = "1970-01-01 00:00:00";
String actTimestamp = "00:00";

unsigned long wpDataHandleCounter = 0;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  // initialize chronos
  wpDataHandleChrono.stop();
  refreshOutputStateChrono.stop();
  
  // initialize default serial interface
  Serial.begin(SERIAL_BAUDRATE);
  Serial.flush();
  delay(SERIAL_DELAY);
  
  // output infoline
  Serial.println("CTA Aeroheat CI 8is Prod [Init WiFi + MQTT]");
  Serial.println();
  
  // initialize WiFi network connection
  Serial.print("WiFi connecting...");
  WiFi.mode(WIFI_STA);  // WiFi station mode
  WiFi.begin(wifiSSID, wifiPassword);
  delay(CONNECTION_DELAY);
  wifiReconnectionCounter = 0;
  while((WiFi.status() != WL_CONNECTED) && (wifiReconnectionCounter < wifiMaxReconnections)) {
    wifiReconnectionCounter++;
    WiFi.reconnect();
    Serial.print("-");
    delay(CONNECTION_RETRY_DELAY);
  }
  if(wifiReconnectionCounter >= wifiMaxReconnections) {
    WiFi.disconnect();
    sprintf(msgString, "\nWiFi Error: WiFi not connected after <%u> retries", wifiReconnectionCounter);
    Serial.println(msgString);
    
    esp32Restart();
  }
  Serial.print("\nWiFi <");
  Serial.print(wifiSSID);
  Serial.print("> / <");
  Serial.print(WiFi.localIP());
  Serial.println("> connected.\n");
  Serial.println();

  // initialize over the air serial monitor
  #ifdef OTA_OUTPUT
    WebSerialPro.begin(&serialOTA);
    WebSerialPro.msgCallback(serialOTAReceiver);
    serialOTA.begin();
    WebSerialPro.setID("WP Prod Home Assistant Integration");
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
    mqttClient.disconnect();
    sprintf(msgString, "\nMQTT Error: MQTT server not connected after <%u> retries", mqttReconnectionCounter);
    Serial.println(msgString);
    
    esp32Restart();
  }
    #ifdef DEBUG_OUTPUT
      Serial.println("\nMQTT server connected.\n");
    #endif
    #ifdef OTA_OUTPUT
      WebSerialPro.print("\nMQTT server connected.\n");
    #endif  
    
  
  if((WiFi.status() == WL_CONNECTED) && mqttClient.connected()) {
    connectionsEstablished = 1;
    
    // MQTT subscriptions
    #ifdef DEBUG_OUTPUT
      Serial.println("CTA Aeroheat CI 8is Prod [MQTT Subscriptions]");
    #endif
    
   // MQTT ESP32 subscription
    mqttTopicID = "ESP32";
    mqttTopicEntity = "Restart";
    mqttTopic = String(mqttTopicID + "/" + mqttTopicEntity).c_str();
    mqttSuccess = mqttClient.subscribe(mqttTopic, mqttQoS);
    #ifdef DEBUG_OUTPUT
      Serial.println("ESP32");
      sprintf(msgString, "%s", String(mqttTopicID + "/" + mqttTopicEntity).c_str());
      Serial.println(msgString);
    #endif
    delay(MQTT_SUBSCRIBE_DELAY);
    
    // MQTT wp converter subscriptions
    #ifdef DEBUG_OUTPUT
      Serial.println("WPConverterStates");
    #endif    

    // MQTT wp system time subscription
    mqttTopicID = "WP";
    mqttTopicEntity = "System_Time";
    mqttTopic = String(mqttTopicID + "/" + mqttTopicEntity).c_str();
    mqttSuccess = mqttClient.subscribe(mqttTopic, mqttQoS);
    #ifdef DEBUG_OUTPUT
      Serial.println("SysTime");
      sprintf(msgString, "%s", String(mqttTopicID + "/" + mqttTopicEntity).c_str());
      Serial.println(msgString);
    #endif
    delay(MQTT_SUBSCRIBE_DELAY);
    
    // MQTT wp register block subscriptions
    for(wpRegisterBlockIndex = 0; wpRegisterBlockIndex < WP_REGISTER_BLOCK_COUNT; wpRegisterBlockIndex++) {
      switch(wpRegisterBlockIndex) {
        case 0: {  // SysValues
          wpRegisterBlockName = "SysValues";
          if(wpRegisterBlocks[wpRegisterBlockIndex].wpRegisterBlockActive) {
            wpRegisterBlockElements = wpRegisterBlocks[wpRegisterBlockIndex].wpRegisterBlockElements;
            
            #ifdef DEBUG_OUTPUT
              sprintf(msgString, "%s", String(wpRegisterBlockName).c_str());
              Serial.println(msgString);
            #endif
            #ifdef OTA_OUTPUT
              sprintf(msgString, "%s", String(wpRegisterBlockName).c_str());
              WebSerialPro.println(msgString);
            #endif
            for(unsigned short element = 0; element < wpRegisterBlockElements; element++) {
              wpEntityDesriptor = wpRegisterSysValues[element].wpRegisterDescriptor;
              wpEntityActiveMQTT = wpRegisterSysValues[element].wpRegisterActiveMQTT;
              
              if(wpEntityActiveMQTT) {
                mqttTopicID = "WP";
                mqttTopic = String(mqttTopicID + "/" + wpEntityDesriptor).c_str();
                mqttSuccess = mqttClient.subscribe(mqttTopic, mqttQoS);
                #ifdef DEBUG_OUTPUT
                  sprintf(msgString, "%s [%u/%u]", String(mqttTopicID + "/" + wpEntityDesriptor).c_str(), wpRegisterBlockIndex, element);
                  Serial.println(msgString);
                #endif
                delay(MQTT_SUBSCRIBE_DELAY);
              }
            }
          }  // end if wpRegisterBlockActive
          //wpRegisterBlockIndex++;
          wpRegisterBlockIndex = 0;  // only one register block
          break;
        }  // end case
        default: {
          wpRegisterBlockIndex = 0;  // reset wp register block index
          break;
        }  // end default
      }  // end switch wpRegisterBlockIndex
    }  // end for wpRegisterBlockIndex
    
    wpRegisterBlockIndex = 0;  // reset wp register block index
  }
   
  // reset reconnection counters
  wifiReconnectionCounter = 0;
  wpReconnectionCounter = 0;
  mqttReconnectionCounter = 0;
  
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
   #ifdef LED_ACTIVE
    if(LEDFlashChrono.hasPassed(LEDFlashInterval)) {
      // flash LED to indicate activity
      stateLED = !(stateLED);
      digitalWrite(LED_BUILTIN, stateLED);
      LEDFlashChrono.restart();
    }
  #endif

  // handle ESP32 restart @06:00
  sprintf(msgString, "%0.2d:%02d", unixTimestamp.hour, unixTimestamp.minute);  // <hh:mm>
  actTimestamp = (String)msgString;

  #ifdef ESP32_RESTART_TIMESTAMP
  // restart only if ESP32_RESTART_TIMESTAMP is defined
    if(actTimestamp == ESP32_RESTART_TIMESTAMP) {
      esp32Restart();
    }
  #endif
  
  // handle MQTT loop
  mqttClient.loop();
  delay(MQTT_LOOP_DELAY);
  
  // output wp converter states
  if(refreshOutputStateChrono.hasPassed(STATE_OUTPUT_REFRESH_INTERVAL_CHRONO)) {
    outputInfoline();
    refreshOutputStateChrono.restart();
  }
  
  if(wpDataHandleChrono.hasPassed(WP_DATA_HANDLE_INTERVAL_CHRONO) || (wpDataHandleCounter == 0)) {
    wpDataHandleCounter++;
    
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
      WebSerialPro.print(msgString);
    #endif

    while(!wifiConnectionID.connect(wpServerIP, wpServerPort) && (wpReconnectionCounter < wpMaxReconnections)) {
      wpReconnectionCounter++;
      #ifdef DEBUG_OUTPUT
        Serial.print("-");
      #endif
      #ifdef OTA_OUTPUT
        WebSerialPro.print("-");
      #endif

      delay(CONNECTION_RETRY_DELAY);
    }
    if(wpReconnectionCounter >= wpMaxReconnections) {
      wifiConnectionID.stop();
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "\nWP Error: WP server not connected after <%u> retries", wpReconnectionCounter);
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "\nWP Error: WP server not connected after <%u> retries", wpReconnectionCounter);
        WebSerialPro.println(msgString);
      #endif
    
      esp32Restart();
    }
    wifiConnectionID.flush();

    #ifdef DEBUG_OUTPUT
      Serial.println("\nWP server connected.\n");
    #endif
    #ifdef OTA_OUTPUT
      WebSerialPro.print("\nWP server connected.\n");
    #endif  
    
    // initalize wp communication (transmit tcp commands '3004' + '0000'
    wifiConnectionID.flush();
    
    tcpTxCounter = tcpTransmitBuffer(wpCommandGetValue);  // tcp transmit '3004'
    if(tcpTxCounter == tcpTxPacketSize) {
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "WP command '3004' <0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x> tranmitted.", wpCommandGetValue[0], wpCommandGetValue[1], wpCommandGetValue[2], wpCommandGetValue[3]);
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "WP command '3004' <0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x> tranmitted.", wpCommandGetValue[0], wpCommandGetValue[1], wpCommandGetValue[2], wpCommandGetValue[3]);
        WebSerialPro.println(msgString);
      #endif  
      delay(TCP_TX_DELAY);
      
      tcpTxCounter = tcpTransmitBuffer(wpCommandExecute);  // tcp transmit '0000'
      if(tcpTxCounter == tcpTxPacketSize) {
        #ifdef DEBUG_OUTPUT
          sprintf(msgString, "WP command '0000' <0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x> tranmitted.\n", wpCommandExecute[0], wpCommandExecute[1], wpCommandExecute[2], wpCommandExecute[3]);
          Serial.println(msgString);
        #endif
        #ifdef OTA_OUTPUT
          sprintf(msgString, "WP command '0000' <0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x> tranmitted.\n", wpCommandExecute[0], wpCommandExecute[1], wpCommandExecute[2], wpCommandExecute[3]);
          WebSerialPro.println(msgString);
        #endif  
        delay(TCP_TX_DELAY);
      }
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
          sprintf(msgString, "WP received <%lu> bytes", tcpRxByteReceived);
          Serial.println(msgString);
        #endif
        #ifdef OTA_OUTPUT
          sprintf(msgString, "WP received <%lu> bytes", tcpRxByteReceived);
          WebSerialPro.println(msgString);
        #endif  
      } else {
        #ifdef DEBUG_OUTPUT
          sprintf(msgString, "\nWP Error: less than <%u> bytes received", wpRxRequestedByteNumber);
          Serial.println(msgString);
        #endif
        #ifdef OTA_OUTPUT
          sprintf(msgString, "\nWP Error: less than <%u> bytes received", wpRxRequestedByteNumber);
          WebSerialPro.println(msgString);
        #endif  
        
        esp32Restart();
      }  // end if tcpRxByteReceived
    } else {
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "\nWP Error: no data received");
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "\nWP Error: no data received");
        WebSerialPro.println(msgString);
      #endif  
      
      esp32Restart();
    }  // end if tcpRxByteAvailable
    
    // check received tcp commands
    tcpCheckGetValue = ((tcpRxBuffer[0] << 24) + (tcpRxBuffer[1] << 16) + (tcpRxBuffer[2] << 8) + tcpRxBuffer[3]);  // bytes 0-3
    tcpCheckExecute = ((tcpRxBuffer[4] << 24) + (tcpRxBuffer[5] << 16) + (tcpRxBuffer[6] << 8) + tcpRxBuffer[7]);  // bytes 4-7
    
    if((tcpCheckGetValue == 3004) && (tcpCheckExecute == 0)) {
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "WP received commands '%0.4lu' + '%0.4lu' valid\n", tcpCheckGetValue, tcpCheckExecute);
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "WP received commands '%0.4lu' + '%0.4lu' valid\n", tcpCheckGetValue, tcpCheckExecute);
        WebSerialPro.println(msgString);
      #endif  
      
      wpCommandTransmitted = 1;
    } else {
      #ifdef DEBUG_OUTPUT
       sprintf(msgString, "\nWP Error: invalid received check commands '%0.4lu' + '%0.4lu'", tcpCheckGetValue, tcpCheckExecute);
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
       sprintf(msgString, "\nWP Error: invalid received check commands '%0.4lu' + '%0.4lu'", tcpCheckGetValue, tcpCheckExecute);
        WebSerialPro.println(msgString);
      #endif  
      
      esp32Restart();
    }   
    
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
      WebSerialPro.print(msgString);
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
        WebSerialPro.print("-");
      #endif
      delay(CONNECTION_RETRY_DELAY);
    }
    if(mqttReconnectionCounter >= mqttMaxReconnections) {
      mqttClient.disconnect();
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "\nMQTT Error: MQTT server not connected after <%u> retries", mqttReconnectionCounter);
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "\nMQTT Error: MQTT server not connected after <%u> retries", mqttReconnectionCounter);
        WebSerialPro.println(msgString);
      #endif

      
      esp32Restart();
    }
    #ifdef DEBUG_OUTPUT
      Serial.println("\nMQTT server connected.\n");
    #endif
    #ifdef OTA_OUTPUT
      WebSerialPro.print("\nMQTT server connected.\n");
    #endif  

    if(wpRegisterBlockIndex == 0) {  // only once per wp register block cycle
      // handle wp system time
      wpRegisterBlockName = "SysTime";
      wpEntityRegisterAddress = wpRegisterBlockSysTime[systenTimeBlockIndex].wpRegisterAddress;
      tcpBufferIndex = (wpEntityRegisterAddress * tcpRxPacketSize);
      
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "%s", String(wpRegisterBlockName).c_str());
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "%s", String(wpRegisterBlockName).c_str());
        WebSerialPro.println(msgString);
      #endif
      wpTimestamp = ((tcpRxBuffer[tcpBufferIndex + 0] << 24) + (tcpRxBuffer[tcpBufferIndex + 1] << 16) + (tcpRxBuffer[tcpBufferIndex + 2] << 8) + tcpRxBuffer[tcpBufferIndex + 3]);
      
      // convert unix time -> date/time (yyyy-mm-dd hh:mm:ss)
      unixTimestamp.getDateTime(wpTimestamp);
      
      mqttTopicID = "WP";
      mqttTopicEntity = "System_Time";
      mqttTopic = String(mqttTopicID + "/" + mqttTopicEntity).c_str();
      
      sprintf(msgString, "%0.4d-%0.2d-%0.2d / %0.2d:%0.2d:%0.2d", unixTimestamp.year, unixTimestamp.month, unixTimestamp.day, unixTimestamp.hour, unixTimestamp.minute, unixTimestamp.second);
      wpTime = msgString;
      //wpTime = String(String(unixTimestamp.year).c_str()).c_str() + "-" + unixTimestamp.month).c_str() + "-" + unixTimestamp.day).c_str() + " " + unixTimestamp.hour).c_str() + ":" + unixTimestamp.minute).c_str() + ":" + unixTimestamp.second).c_str()).c_str();
      mqttPayload = ("{\"" + String(mqttTopicEntity) + "\":\"" + String(wpTime) + "\"}").c_str();
      mqttSuccess = mqttClient.publish(mqttTopic, mqttPayload, mqttRetained, mqttQoS);
      
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "%s: ", String(mqttTopicEntity).c_str());
        Serial.print(msgString);
        Serial.println(String(wpTime).c_str());
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "%s: ", String(mqttTopicEntity).c_str());
        WebSerialPro.print(msgString);
        WebSerialPro.println(String(wpTime).c_str());
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
            sprintf(msgString, "%s", String(wpRegisterBlockName).c_str());
            Serial.println(msgString);
          #endif
          #ifdef OTA_OUTPUT
            sprintf(msgString, "%s", String(wpRegisterBlockName).c_str());
            WebSerialPro.println(msgString);
          #endif
          
          mqttTopicID = "WP";
          
          for(unsigned short element = 0; element < wpRegisterBlockElements; element++) {
            wpEntityRegisterAddress = wpRegisterSysValues[element].wpRegisterAddress;
            wpEntityDesriptor = wpRegisterSysValues[element].wpRegisterDescriptor;
            wpEntityDataType = wpRegisterSysValues[element].wpRegisterDataType;
            wpEntityDataFactor = wpRegisterSysValues[element].wpRegisterDataFactor;
            wpEntityUnit = wpRegisterSysValues[element].wpRegisterUnit;
            wpEntityActiveMQTT = wpRegisterSysValues[element].wpRegisterActiveMQTT;
            
            tcpBufferIndex = (wpEntityRegisterAddress * tcpRxPacketSize);  // (wpEntity...Address * 4), ((wpEntity...Address + 1) * 4), ((wpEntity...Address + 1) * 4), ..., (wpRegisterBlockElements * 4)
            
            if(wpEntityRegisterAddress == WP_REGISTER_VALUES_STATE_WP) {  // special case values wp state
              uint32_t wpValueU32 = ((tcpRxBuffer[tcpBufferIndex + 0] << 24) + (tcpRxBuffer[tcpBufferIndex + 1] << 16) + (tcpRxBuffer[tcpBufferIndex + 2] << 8) + tcpRxBuffer[tcpBufferIndex + 3]);
              
              wpStateID = wpValueU32;  // global wp state ID
              wpState = wpRegisterSysValuesState[wpStateID].wpRegisterStateDescriptor;  // global wp state
            }
            
            outputWPData(tcpRxBuffer, tcpBufferIndex, wpEntityDesriptor, wpEntityDataType, wpEntityDataFactor, wpEntityUnit, wpEntityActiveMQTT, mqttRetained, mqttQoS);
          }
          wpDataHandleChrono.restart();  // restart chrono
          
          #ifdef DEBUG_OUTPUT
            Serial.println();
          #endif
          #ifdef OTA_OUTPUT
            WebSerialPro.println();
          #endif
        }  // end if wpRegisterBlockActive
        //wpRegisterBlockIndex++;
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
void esp32Restart() {
  String mqttTopicID = "ESP32";
  String mqttTopicEntity = "Restart";
  String mqttPayload="1";
  bool mqttSuccess;
  
  Serial.println("ESP32 Restarting...");
  #ifdef OTA_OUTPUT
    WebSerialPro.println("ESP32 Restarting...");
  #endif
  
  // MQTT publish
  mqttTopic = String(mqttTopicID + "/" + mqttTopicEntity).c_str();
  mqttPayload = ("{\"" + String(mqttTopicEntity) + "\":\"" + String(mqttPayload) + "\"}").c_str();
  mqttSuccess = mqttClient.publish(mqttTopic, mqttPayload, mqttRetained, mqttQoS);
  delay(MQTT_PUBLISH_DELAY);
  
  // reset connections
  mqttClient.disconnect();
  delay(CONNECTION_DELAY);
  wifiConnectionID.stop();
  delay(CONNECTION_DELAY);
  WiFi.disconnect();
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
    WebSerialPro.println("ESP32 Restarting...");
    delay(ESP32_RESTART_DELAY);
    esp32Restart();
  } else if(buffer == "Halt") {
    WebSerialPro.println("ESP32 Halt...");
    delay(ESP32_RESTART_DELAY);
    while(1);
  } else {
    WebSerialPro.println("Usage: \'Restart\' | \'Halt\'");
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
    sprintf(msgString, "WP CTA Aeroheat CI 8is Prod [%s] - %s", String(wpState).c_str(), String(wpTime).c_str());
    Serial.println(msgString);
    
    Serial.println();
  #endif
  
  #ifdef OTA_OUTPUT
    sprintf(msgString, "WP CTA Aeroheat CI 8is Prod [%s] - %s", String(wpState).c_str(), String(wpTime).c_str());
    WebSerialPro.println(msgString);
    
    WebSerialPro.println();
  #endif
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void outputWPData(uint8_t *tcpBuffer, uint16_t tcpBufferIndex, char *wpEntityDesriptor, char *wpEntityDataType, float wpEntityDataFactor, char *wpEntityUnit, bool mqttActive, bool mqttRetained, uint16_t mqttQoS) {
  String mqttTopicID = "WP";
  bool mqttSuccess;
  
  if((wpEntityDataType == "U32") || (wpEntityDataType == "Status") || (wpEntityDataType == "I/O")) {
    uint32_t bufferValueUInt32 = ((tcpBuffer[tcpBufferIndex + 0] << 24) + (tcpBuffer[tcpBufferIndex + 1] << 16) + (tcpBuffer[tcpBufferIndex + 2] << 8) + tcpBuffer[tcpBufferIndex + 3]);
    
    #ifdef DEBUG_OUTPUT
      sprintf(msgString, "%s: %u%s", String(wpEntityDesriptor).c_str(), bufferValueUInt32, String(wpEntityUnit).c_str());
      Serial.println(msgString);
    #endif
    #ifdef OTA_OUTPUT
      sprintf(msgString, "%s: %u%s", String(wpEntityDesriptor).c_str(), bufferValueUInt32, String(wpEntityUnit).c_str());
      WebSerialPro.println(msgString);
    #endif
    
    if(mqttActive) {
      mqttSuccess = mqttPublishTopicPayloadUInt32(mqttTopicID, wpEntityDesriptor, bufferValueUInt32, mqttRetained, mqttQoS);
    }
  } else if(wpEntityDataType == "F32") {
    // float convertion method #1
    float bufferValueFloat32 = (((tcpBuffer[tcpBufferIndex + 0] << 24) + (tcpBuffer[tcpBufferIndex + 1] << 16) + (tcpBuffer[tcpBufferIndex + 2] << 8) + tcpBuffer[tcpBufferIndex + 3]) * wpEntityDataFactor);
    
    if(wpEntityUnit == "h") {
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "%s: %0.2f%s", String(wpEntityDesriptor).c_str(), bufferValueFloat32, String(wpEntityUnit).c_str());
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "%s: %0.2f%s", String(wpEntityDesriptor).c_str(), bufferValueFloat32, String(wpEntityUnit).c_str());
        WebSerialPro.println(msgString);
      #endif
    } else {
      #ifdef DEBUG_OUTPUT
        sprintf(msgString, "%s: %0.2f%s", String(wpEntityDesriptor).c_str(), bufferValueFloat32, String(wpEntityUnit).c_str());
        Serial.println(msgString);
      #endif
      #ifdef OTA_OUTPUT
        sprintf(msgString, "%s: %0.2f%s", String(wpEntityDesriptor).c_str(), bufferValueFloat32, String(wpEntityUnit).c_str());
        WebSerialPro.println(msgString);
      #endif
    }

    if(mqttActive) {
      mqttSuccess = mqttPublishTopicPayloadFloat32(mqttTopicID, wpEntityDesriptor, bufferValueFloat32, mqttRetained, mqttQoS);
    }
  } else {
    #ifdef DEBUG_OUTPUT
      Serial.print("Unknown Type: ");
      Serial.println(wpEntityDataType);
    #endif
    #ifdef OTA_OUTPUT
      WebSerialPro.print("Unknown Type:");
      WebSerialPro.println(wpEntityDataType);
    #endif
  }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
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