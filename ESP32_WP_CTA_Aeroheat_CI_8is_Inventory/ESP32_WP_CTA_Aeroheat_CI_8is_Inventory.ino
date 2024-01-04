/*
  ESP32: WP CTA Aeroheat CI 8is Inventory
*/

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <Arduino.h>
#include <ESP32.h>

#include <Chrono.h>
#include <WiFi.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerialPro.h>

#include "ESP32_WP_CTA_Aeroheat_CI_8is_Inventory.h"

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#define SERIAL_BAUDRATE 115200  // default serial interface bauderate
#define SERIAL_DELAY 200        // serial communication delay [ms]

#define CONNECTION_DELAY 1000        // WiFi / connection delay [ms]
#define CONNECTION_RETRY_DELAY 1000  // WiFi / retry connection delay [ms]

#define TCP_COMMUNICATION_DELAY 1000  // WP TCP communication delay [ms]
#define TCP_RX_DELAY 50               // WP TCP receive delay [ms]
#define TCP_TX_DELAY 50               // WP TCP transmit delay [ms]

#define WP_DATA_HANDLE_INTERVAL_CHRONO 20  // chrono wp register data handle interval [s]

#define DEBUG_OUTPUT  // default serial interface output active

#define OTA_OUTPUT  // over the air serial monitor output active

#define ESP32_RESTART_DELAY 5000  // ESP32 restart command delay [ms]

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// instantiate chrono objects
Chrono wpDataHandleChrono(Chrono::SECONDS);

// instantiate WiFi objects
WiFiClient wifiConnectionID;

// instantiate over the air serial monitor
#ifdef OTA_OUTPUT
  AsyncWebServer serialOTA(80);
#endif

// WiFi definitions
const char wifiSSID[] = "WLAN_Belp";  // WiFi network SSID
const char wifiPassword[] = "family.ruch@belp";  // WiFi network password

unsigned int wifiReconnectionCounter = 0;
const unsigned int wifiMaxReconnections = 10;

// CTA Aeroheat CI 8is Inventory definitions
const char wpServerIP[] = "192.168.1.190";
int wpServerPort = 8889;
const char wpUser[] = "";
const char wpPassword[] = "666666";

unsigned long tcpTxCounter;
unsigned long tcpRxCounter;

uint16_t wpRegisterBlockHeadElement;
uint16_t wpRegisterBlockElements;

// communication definitions
unsigned int wpReconnectionCounter = 0;
const unsigned int wpMaxReconnections = 10;

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

// common definitions
char msgString[256];  // message char array

String wpRegisterBlockName;
unsigned int wpRegisterBlockIndex = 0;
unsigned int tcpBufferIndex;

unsigned long wpDataHandleCounter = 0;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  // initialize chronos
  wpDataHandleChrono.stop();
  
  // initialize default serial interface
  Serial.begin(SERIAL_BAUDRATE);
  Serial.flush();
  delay(SERIAL_DELAY);
  
  // output infoline
  Serial.println("CTA Aeroheat CI 8is Inventory [Init WiFi]");
  Serial.println();
  
  // initialize WiFi network connection
  Serial.print("Connecting WiFi...");
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
  Serial.print("\nWiFi connected: ");
  Serial.print(wifiSSID);
  Serial.print(" / ");
  Serial.println(WiFi.localIP());
  Serial.println();
  
  // initialize WP server connection
  Serial.print("Connecting WP server <");
  Serial.print(wpServerIP);
  Serial.print(":");
  Serial.print(wpServerPort);
  Serial.print(">...");
  wpReconnectionCounter = 0;
  while(!wifiConnectionID.connect(wpServerIP, wpServerPort) && (wpReconnectionCounter < wpMaxReconnections)) {
    wpReconnectionCounter++;
    Serial.print("-");
    delay(CONNECTION_RETRY_DELAY);
  }
  if(wpReconnectionCounter >= wpMaxReconnections) {
    wifiConnectionID.stop();
    sprintf(msgString, "\nWP Error: WP server not connected after <%u> retries", wpReconnectionCounter);
    Serial.println(msgString);
    
    esp32Restart();
  }
  wifiConnectionID.flush();
  
  Serial.println("\nWP server connected.");
  Serial.println();

  // initialize over the air serial monitor
  #ifdef OTA_OUTPUT
    WebSerialPro.begin(&serialOTA);
    WebSerialPro.msgCallback(serialOTAReceiver);
    serialOTA.begin();
    WebSerialPro.setID("CTA Aeroheat CI 8is Inventory Home Assistant Integration");
  #endif
  
  wpRegisterBlockIndex = 0;  // reset wp register block index
  
  // reset reconnection counters
  wifiReconnectionCounter = 0;
  wpReconnectionCounter = 0;
  
  // start chronos
  wpDataHandleChrono.start();
  
  #ifdef DEBUG_OUTPUT
    Serial.println();
  #endif
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
 
  if(wpDataHandleChrono.hasPassed(WP_DATA_HANDLE_INTERVAL_CHRONO) || (wpDataHandleCounter == 0)) {
    wpDataHandleCounter++;
    
    // initalize wp communication (transmit tcp commands '3004' + '0000'
    wifiConnectionID.flush();
    
    tcpTxCounter = tcpTransmitBuffer(wpCommandGetValue);  // tcp transmit '3004'
    if(tcpTxCounter == tcpTxPacketSize) {
      sprintf(msgString, "WP Command '3004' transmitted: 0x%.2x 0x%.2x 0x%.2x 0x%.2x", wpCommandGetValue[0], wpCommandGetValue[1], wpCommandGetValue[2], wpCommandGetValue[3]);
      Serial.println(msgString);
      delay(TCP_TX_DELAY);
      
      tcpTxCounter = tcpTransmitBuffer(wpCommandExecute);  // tcp transmit '0000'
      if(tcpTxCounter == tcpTxPacketSize) {
        sprintf(msgString, "WP Command '0000' transmitted: 0x%.2x 0x%.2x 0x%.2x 0x%.2x", wpCommandExecute[0], wpCommandExecute[1], wpCommandExecute[2], wpCommandExecute[3]);
        Serial.println(msgString);
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
        sprintf(msgString, "TCP received bytes: <%lu>\n", tcpRxByteReceived);
        Serial.println(msgString);
      } else {
        sprintf(msgString, "\nWP Error: less than <%u> bytes received", wpRxRequestedByteNumber);
        Serial.println(msgString);
        
        esp32Restart();
      }  // end if tcpRxByteReceived
    } else {
      sprintf(msgString, "\nWP Error: no data received");
      Serial.println(msgString);
      
      esp32Restart();
    }  // end if tcpRxByteAvailable
    
    // check received tcp commands
    tcpCheckGetValue = ((tcpRxBuffer[0] << 24) + (tcpRxBuffer[1] << 16) + (tcpRxBuffer[2] << 8) + tcpRxBuffer[3]);  // bytes 0-3
    tcpCheckExecute = ((tcpRxBuffer[4] << 24) + (tcpRxBuffer[5] << 16) + (tcpRxBuffer[6] << 8) + tcpRxBuffer[7]);  // bytes 4-7
    
    if((tcpCheckGetValue == 3004) && (tcpCheckExecute == 0)) {
      sprintf(msgString, "WP received commands valid <%lu | %lu>", tcpCheckGetValue, tcpCheckExecute);
      Serial.println(msgString);
      
      wpCommandTransmitted = 1;
    } else {
      sprintf(msgString, "\nWP Error: invalid received check commands <%lu | %lu>", tcpCheckGetValue, tcpCheckExecute);
      Serial.println(msgString);
      
      esp32Restart();
    }
  
    // handle wp data
    switch(wpRegisterBlockIndex) {
      case 0: {  // Values
        wpRegisterBlockName = "Values";
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
          
          for(unsigned short element = wpRegisterBlockHeadElement; element < (wpRegisterBlockElements + wpRegisterBlockHeadElement); element ++) {
            tcpBufferIndex = (element * tcpRxPacketSize);  // (0 * 4), (1 * 4), (2 * 4), ..., (wpRegisterBlockElements * 4)            
            // interger convertion
            int32_t wpValueI32 = ((tcpRxBuffer[tcpBufferIndex + 0] << 24) + (tcpRxBuffer[tcpBufferIndex + 1] << 16) + (tcpRxBuffer[tcpBufferIndex + 2] << 8) + tcpRxBuffer[tcpBufferIndex + 3]);
            uint32_t wpValueU32 = ((tcpRxBuffer[tcpBufferIndex + 0] << 24) + (tcpRxBuffer[tcpBufferIndex + 1] << 16) + (tcpRxBuffer[tcpBufferIndex + 2] << 8) + tcpRxBuffer[tcpBufferIndex + 3]);
            
            // float convertion method #1
            float wpValueF32a = ((tcpRxBuffer[tcpBufferIndex + 0] << 24) + (tcpRxBuffer[tcpBufferIndex + 1] << 16) + (tcpRxBuffer[tcpBufferIndex + 2] << 8) + tcpRxBuffer[tcpBufferIndex + 3]);
            
            // float convertion method #2
            union {
              float float32;
              uint32_t uint32;
            } conversion;
            float wpValueF32b;
            
            conversion.uint32 = ((tcpRxBuffer[tcpBufferIndex + 0] << 24) + (tcpRxBuffer[tcpBufferIndex + 1] << 16) + (tcpRxBuffer[tcpBufferIndex + 2] << 8) + tcpRxBuffer[tcpBufferIndex + 3]);
            wpValueF32b = conversion.float32;
            
            int32_t wpValueI32c = ((tcpRxBuffer[tcpBufferIndex + 0] << 24) + (tcpRxBuffer[tcpBufferIndex + 1] << 16) + (tcpRxBuffer[tcpBufferIndex + 2] << 8) + tcpRxBuffer[tcpBufferIndex + 3]);
            float wpValueF32c = float(wpValueI32c);
            
            sprintf(msgString, "%u (%u) | ", element, wpRegisterBlockElements);
            Serial.print(msgString);
            sprintf(msgString, "#%.2u[%.2u]: 0x%.2x 0x%.2x 0x%.2x 0x%.2x", wpRegisterBlockIndex, tcpBufferIndex, tcpRxBuffer[tcpBufferIndex + 0], tcpRxBuffer[tcpBufferIndex + 1], tcpRxBuffer[tcpBufferIndex + 2], tcpRxBuffer[tcpBufferIndex + 3]);
            Serial.print(msgString);
            sprintf(msgString, " | %ld", wpValueI32);
            Serial.print(msgString);
            sprintf(msgString, " | %lu", wpValueU32);
            Serial.print(msgString);
            sprintf(msgString, " | %0.1f", wpValueF32a);
            Serial.print(msgString);
            sprintf(msgString, " | %0.1f", wpValueF32b);
            Serial.print(msgString);
            sprintf(msgString, " | %0.1f", wpValueF32c);
            Serial.print(msgString);
            /*
            sprintf(msgString, " | %c %c %c %c", char(tcpRxBuffer[tcpBufferIndex + 0]), char(tcpRxBuffer[tcpBufferIndex + 1]), char(tcpRxBuffer[tcpBufferIndex + 2]), char(tcpRxBuffer[tcpBufferIndex + 3]));
            Serial.print(msgString);
            sprintf(msgString, " | %s %s %s %s", String(tcpRxBuffer[tcpBufferIndex + 0]), String(tcpRxBuffer[tcpBufferIndex + 1]), String(tcpRxBuffer[tcpBufferIndex + 2]), String(tcpRxBuffer[tcpBufferIndex + 3]));
            Serial.print(msgString);
            Serial.print(" | ");
            Serial.print(char(tcpRxBuffer[tcpBufferIndex + 0]));
            Serial.print(" ");
            Serial.print(char(tcpRxBuffer[tcpBufferIndex + 1]));
            Serial.print(" ");
            Serial.print(char(tcpRxBuffer[tcpBufferIndex + 2]));
            Serial.print(" ");
            Serial.print(char(tcpRxBuffer[tcpBufferIndex + 3]));
            Serial.print();
            */
            Serial.println();
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
  Serial.println("ESP32 Restarting...");
  #ifdef OTA_OUTPUT
    WebSerialPro.println("ESP32 Restarting...");
  #endif
  
  // reset connections
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

//EOF