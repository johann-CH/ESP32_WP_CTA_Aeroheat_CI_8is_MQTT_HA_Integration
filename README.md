## WP CTA Aeroheat CI 8is Prod Home Assistant Integration (MQTT)
Heat Pump CTA Aeroheat CI 8is: ESP32 MQTT Converter + Home Assistant Integration

### Description:
The code is tested with a **CTA Aeroheat CI 8is**. The **CTA Aeroheat CI 8is** is a rebranded model **Alpha Innotec LWCV 82R1/3**, so the code should also work for this type.


### Hardware Prerequisites:
- ESP32 Board with WiFi
- LAN Connection: Luxtronic Ethernet Interface/LAN

### Software Prerequisites:
- Arduino IDE or other Development Tool
- Libraries
  - <NTP.h>
  - <UnixTime.h>
  - <AsyncTCP.h>
  - <ESPAsyncWebServer.h>
  - <WebSerial.h> or licensed <WebSerialPro.h>
  - <MQTT.h>