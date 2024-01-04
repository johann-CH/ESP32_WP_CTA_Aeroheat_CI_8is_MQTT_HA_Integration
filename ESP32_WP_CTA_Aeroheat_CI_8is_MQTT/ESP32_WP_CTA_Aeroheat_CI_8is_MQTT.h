/*
  ESP32_WP_CTA_Aeroheat_CI_8is_MQTT.h
*/

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// wp register blocks
#define WP_REGISTER_BLOCK_VALUES 0

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// wp values [137] / register block SysTime
#define WP_REGISTER_VALUES_SYSTIME_TIMESTAMP_AKT  137  // # 0 [548]

// wp values [13-65] / register block #0
#define WP_REGISTER_VALUES_NUMBER_BYTES        1064
#define WP_REGISTER_VALUES_HEAD_ELEMENT        13
#define WP_REGISTER_VALUES_ELEMENTS            18  // max. 70
#define WP_REGISTER_VALUES_ACTIVE              1
#define WP_REGISTER_VALUES_TEMP_VL             13   // # 0 [52]
#define WP_REGISTER_VALUES_TEMP_RL             14   // # 1 [56]
#define WP_REGISTER_VALUES_TEMP_RL_SOLL        15   // # 2 [60]
#define WP_REGISTER_VALUES_TEMP_RL_EXT         16   // # 3 [64]
#define WP_REGISTER_VALUES_TEMP_HG             17   // # 4 [68]
#define WP_REGISTER_VALUES_TEMP_AUSSEN         18   // # 5 [72]
#define WP_REGISTER_VALUES_TEMP_MITTEL         19   // # 6 [76]
#define WP_REGISTER_VALUES_TEMP_WQ             22   // # 7 [88]
#define WP_REGISTER_VALUES_IO_ASD              32   // # 8 [128]
#define WP_REGISTER_VALUES_IO_EVU              34   // # 9 [136]
#define WP_REGISTER_VALUES_IO_HD               35   // # 10 [140]
#define WP_REGISTER_VALUES_IO_MOTOR            36   // # 11 [144]
#define WP_REGISTER_VALUES_IO_ND               37   // # 12 [148]
#define WP_REGISTER_VALUES_IO_PEX              38   // # 13 [152]
#define WP_REGISTER_VALUES_IO_SWT              39   // # 14 [156]
#define WP_REGISTER_VALUES_IO_ABTAUVENTIL      40   // # 15 [160]
#define WP_REGISTER_VALUES_IO_BUP              41   // # 16 [164]
#define WP_REGISTER_VALUES_IO_HUB              42   // # 17 [168]
#define WP_REGISTER_VALUES_IO_VENTILATOR       45   // # 18 [180]
#define WP_REGISTER_VALUES_IO_VENTILATOR_BOSUP 46   // # 19 [184]
#define WP_REGISTER_VALUES_IO_VD1              47   // # 20 [188]
#define WP_REGISTER_VALUES_IO_ZIP              49   // # 21 [196]
#define WP_REGISTER_VALUES_IP_ZUP              50   // # 22 [200]
#define WP_REGISTER_VALUES_IO_ZWE1             51   // # 23 [204]
#define WP_REGISTER_VALUES_IO_FUP2             54   // # 24 [216]
#define WP_REGISTER_VALUES_IO_SLP              55   // # 25 [220]
#define WP_REGISTER_VALUES_IO_SUP              56   // # 26 [224]
#define WP_REGISTER_VALUES_HH_VD1              59   // # 27 [236]
#define WP_REGISTER_VALUES_IMPULS_VD1          60   // # 28 [240]
#define WP_REGISTER_VALUES_HH_WP               66   // # 29 [264]
#define WP_REGISTER_VALUES_HH_HEIZUNG          67   // # 30 [268]
#define WP_REGISTER_VALUES_HH_WP_EIN           70   // # 31 [280]
#define WP_REGISTER_VALUES_STAT_BV             82   // # 32 [328]
#define WP_REGISTER_VALUES_STATE_WP            83   // # 33 [332]
#define WP_REGISTER_VALUES_ERR_ZEIT0           98   // # 34 [392]
#define WP_REGISTER_VALUES_ERR_ZEIT1           99   // # 35 [396]
#define WP_REGISTER_VALUES_ERR_ZEIT2           100  // # 36 [400]
#define WP_REGISTER_VALUES_ERR_ZEIT3           101  // # 37 [404]
#define WP_REGISTER_VALUES_ERR_ZEIT4           102  // # 38 [408]
#define WP_REGISTER_VALUES_ERR_NR0             103  // # 39 [412]
#define WP_REGISTER_VALUES_ERR_NR1             104  // # 40 [416]
#define WP_REGISTER_VALUES_ERR_NR2             105  // # 41 [420]
#define WP_REGISTER_VALUES_ERR_NR3             106  // # 42 [424]
#define WP_REGISTER_VALUES_ERR_NR4             107  // # 43 [428]
#define WP_REGISTER_VALUES_ERR_COUNT           108  // # 44 [432]
#define WP_REGISTER_VALUES_HH_ABTAUEN          144  // # 45 [576]
#define WP_REGISTER_VALUES_ENERGIE_WP          154  // # 46 [616]
#define WP_REGISTER_VALUES_ENERGIE_RESET       157  // # 47 [628]
#define WP_REGISTER_VALUES_IO_OUT1             159  // # 48 [636]
#define WP_REGISTER_VALUES_IO_OUT2             160  // # 49 [640]
#define WP_REGISTER_VALUES_DURCHFLUSS_WMZ      176  // # 50 [704]
#define WP_REGISTER_VALUES_TEMP_VERDAMPF       178  // # 51 [712]
#define WP_REGISTER_VALUES_TEMP_ANSAUG_VD      179  // # 52 [716]
#define WP_REGISTER_VALUES_TEMP_VD_HEIZUNG     180  // # 53 [720]
#define WP_REGISTER_VALUES_TEMP_UEBERHITZ      181  // # 54 [724]
#define WP_REGISTER_VALUES_TEMP_UEBERHITZ_SOLL 182  // # 55 [728]
#define WP_REGISTER_VALUES_DRUCK_HD            183  // # 56 [732]
#define WP_REGISTER_VALUES_DRUCK_ND            184  // # 57 [736]
#define WP_REGISTER_VALUES_IO_VD_HEIZUNG       185  // # 58 [740]
#define WP_REGISTER_VALUES_RPM_UMWAELZPUMPE    186  // # 59 [744]
#define WP_REGISTER_VALUES_RPM_VENTI           187  // # 60 [748]
#define WP_REGISTER_VALUES_TEMP_VL_SOLL        192  // # 61 [768]
#define WP_REGISTER_VALUES_RPM_VD1             234  // # 62 [936]
#define WP_REGISTER_VALUES_RPM_VD1_SOLL        239  // # 63 [956]
#define WP_REGISTER_VALUES_TEMP_FL1            252  // # 64 [1008]
#define WP_REGISTER_VALUES_TEMP_FL2            253  // # 65 [1012]
#define WP_REGISTER_VALUES_DURCHFLUSS          257  // # 66 [1028]
#define WP_REGISTER_VALUES_LEISTUNG_HEIZUNG    260  // # 67 [1040]
#define WP_REGISTER_VALUES_FREIGABE_KUEHLUNG   263  // # 68 [1052]

#define WP_REGISTER_BLOCK_COUNT 1  // number of wp register blocks

// wp state code (WP_REGISTER_VALUES_STATE_WP)
#define WP_STATE_HEIZEN          0x00
#define WP_STATE_WARMWASSER      0x01
#define WP_STATE_AUSHEIZPROGRAMM 0x02
#define WP_STATE_ABTAU           0x03
#define WP_STATE_EVU             0x04
#define WP_STATE_PUMPENVORLAUF   0x05
#define WP_STATE_FEHLER          0x06
#define WP_STATE_OFFLINE         0x07

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
struct wpRegisterBlockStruct {
  uint16_t wpRegisterBlockHeadElement;
  uint16_t wpRegisterBlockElements;
  uint16_t wpRegisterBlockActive;
};

static struct wpRegisterBlockStruct wpRegisterBlocks[] = {
  { WP_REGISTER_VALUES_HEAD_ELEMENT, WP_REGISTER_VALUES_ELEMENTS, WP_REGISTER_VALUES_ACTIVE }  // #0
};

struct wpRegisterSysTimeStruct {
  uint16_t wpRegisterAddress;
};

static struct wpRegisterSysTimeStruct wpRegisterBlockSysTime[] = {  // register block SysTime
  {WP_REGISTER_VALUES_SYSTIME_TIMESTAMP_AKT}                        // #0
};

struct wpRegisterDataStruct {
  uint16_t wpRegisterAddress;
  char *wpRegisterDescriptor;
  char *wpRegisterDataType;
  float wpRegisterDataFactor;
  char *wpRegisterUnit;
  bool wpRegisterActiveMQTT;
};

static struct wpRegisterDataStruct wpRegisterSysValues[] = {                                             // register block #0
  {WP_REGISTER_VALUES_TEMP_VL, "Temperatur_Vorlauf", "F32", 0.1, "°C", 1 },                              // #0
  {WP_REGISTER_VALUES_TEMP_RL, "Temperatur_Rücklauf", "F32", 0.1, "°C", 1 },                             // #1
  {WP_REGISTER_VALUES_TEMP_RL_SOLL, "Temperatur_Rücklauf_Soll", "F32", 0.1, "°C", 1 },                   // #2
  //{WP_REGISTER_VALUES_TEMP_RL_EXT, "Temperatur_externer_Rücklauffühler", "F32", 0.1, "°C", 0 },        // #3
  //{WP_REGISTER_VALUES_TEMP_HG, "Temperatur_Heissgas", "F32", 0.1, "°C", 1 },                             // #4
  {WP_REGISTER_VALUES_TEMP_AUSSEN, "Temperatur_Aussenfühler", "F32", 0.1, "°C", 1 },                     // #5
  {WP_REGISTER_VALUES_TEMP_MITTEL, "Temperatur_Aussenfühler_Durchschnitt_24h", "F32", 0.1, "°C", 1 },    // #6
  //{WP_REGISTER_VALUES_TEMP_WQ, "Temperatur_Wärmequellenfühler_Eintritt", "F32", 0.1, "°C", 1 },          // #7
  //{WP_REGISTER_VALUES_IO_ASD, "ASD_I/O", "I/O", 1.0, "", 0 },                                            // #8
  //{WP_REGISTER_VALUES_IO_EVU, "EVU_I/O", "I/O", 1.0, "", 0 },                                            // #9
  //{WP_REGISTER_VALUES_IO_HD, "HD_I/O", "I/O", 1.0, "", 0 },                                              // #10
  //{WP_REGISTER_VALUES_IO_MOTOR, "MOTOR_I/O", "I/O", 1.0, "", 0 },                                        // #11
  //{WP_REGISTER_VALUES_IO_ND, "ND_I/O", "I/O", 1.0, "", 0 },                                              // #12
  //{WP_REGISTER_VALUES_IO_PEX, "PEX_I/O", "I/O", 1.0, "", 0 },                                            // #13
  //{WP_REGISTER_VALUES_IO_SWT, "SWT_I/O", "I/O", 1.0, "", 0 },                                            // #14
  //{WP_REGISTER_VALUES_IO_ABTAUVENTIL, "Abtauventil_I/O", "I/O", 1.0, "", 0 },                            // #15
  //{WP_REGISTER_VALUES_IO_BUP, "BUP_I/O", "I/O", 1.0, "", 0 },                                            // #16
  //{WP_REGISTER_VALUES_IO_HUB, "HUP_I/O", "I/O", 1.0, "", 0 },                                            // #17
  //{WP_REGISTER_VALUES_IO_VENTILATOR, "Ventilator_I/O", "I/O", 1.0, "", 0 },                              // #18
  //{WP_REGISTER_VALUES_IO_VENTILATOR_BOSUP, "Ventilator_BOSUP_I/O", "I/O", 1.0, "", 0 },                  // #19
  //{WP_REGISTER_VALUES_IO_VD1, "VD1_I/O", "I/O", 1.0, "", 0 },                                            // #20
  //{WP_REGISTER_VALUES_IO_ZIP, "ZIP_I/O", "I/O", 1.0, "", 0 },                                            // #21
  //{WP_REGISTER_VALUES_IP_ZUP, "ZUP_I/O", "I/O", 1.0, "", 0 },                                            // #22
  //{WP_REGISTER_VALUES_IO_ZWE1, "ZWE1_I/O", "I/O", 1.0, "", 0 },                                          // #23
  //{WP_REGISTER_VALUES_IO_FUP2, "FUP2_I/O", "I/O", 1.0, "", 0 },                                          // #24
  //{WP_REGISTER_VALUES_IO_SLP, "SLP_I/O", "I/O", 1.0, "", 0 },                                            // #25
  //{WP_REGISTER_VALUES_IO_SUP, "SUP_I/O", "I/O", 1.0, "", 0 },                                            // #26
  //{WP_REGISTER_VALUES_HH_VD1, "Betriebsstunden_Verdichter", "F32", 0.000277778, "h", 1 },                // #27
  {WP_REGISTER_VALUES_IMPULS_VD1, "Impulse_Verdichter", "U32", 1.0, "", 1 },                             // #28
  {WP_REGISTER_VALUES_HH_WP, "Betriebsstunden", "F32", 0.000277778, "h", 1 },                            // #29
  //{WP_REGISTER_VALUES_HH_HEIZUNG, "Betriebsstunden_Heizung", "F32", 0.000277778, "h", 1 },               // #30
  {WP_REGISTER_VALUES_HH_WP_EIN, "Einschaltdauer", "F32", 0.000277778, "h", 1 },                         // #31
  //{WP_REGISTER_VALUES_STAT_BV, "Bivalenz_Stufe", "Status", 1.0, "", 1 },                                 // #32
  {WP_REGISTER_VALUES_STATE_WP, "Betriebszustand", "Status", 1.0, "", 1 },                               // #33
  //{WP_REGISTER_VALUES_ERR_ZEIT0, "Error_Zeit0", "U32", 1.0, "", 0 },                                     // #34
  //{WP_REGISTER_VALUES_ERR_ZEIT1, "Error_Zeit1", "U32", 1.0, "", 0 },                                     // #35
  //{WP_REGISTER_VALUES_ERR_ZEIT2, "Error_Zeit2", "U32", 1.0, "", 0 },                                     // #36
  //{WP_REGISTER_VALUES_ERR_ZEIT3, "Error_Zeit3", "U32", 1.0, "", 0 },                                     // #37
  //{WP_REGISTER_VALUES_ERR_ZEIT4, "Error_Zeit4", "U32", 1.0, "", 0 },                                     // #38
  //{WP_REGISTER_VALUES_ERR_NR0, "Error_Nr0", "U32", 1.0, "", 0 },                                         // #39
  //{WP_REGISTER_VALUES_ERR_NR1, "Error_Nr1", "U32", 1.0, "", 0 },                                         // #40
  //{WP_REGISTER_VALUES_ERR_NR2, "Error_Nr2", "U32", 1.0, "", 0 },                                         // #41
  //{WP_REGISTER_VALUES_ERR_NR3, "Error_Nr3", "U32", 1.0, "", 0 },                                         // #42
  //{WP_REGISTER_VALUES_ERR_NR4, "Error_Nr4", "U32", 1.0, "", 0 },                                         // #43
  //{WP_REGISTER_VALUES_ERR_COUNT, "Fehlerspeicher", "U32", 1.0, "", 1 },                                  // #44
  {WP_REGISTER_VALUES_HH_ABTAUEN, "Abtauen", "F32", 0.000277778, "h", 1 },                               // #46
  {WP_REGISTER_VALUES_ENERGIE_WP, "Wärmemenge_Heizung", "F32", 0.1, "kWh", 1 },                          // #47
  //{WP_REGISTER_VALUES_ENERGIE_RESET, "Wärmemenge_seit_Reset", "F32", 0.1, "kWh", 0 },                    // #48
  //{WP_REGISTER_VALUES_IO_OUT1, "Analog_Out1", "F32", 0.01, "V", 0 },                                     // #49
  //{WP_REGISTER_VALUES_IO_OUT2, "Analog_Out2", "F32", 0.01, "V", 0 },                                     // #50
  {WP_REGISTER_VALUES_DURCHFLUSS_WMZ, "Durchfluss_Wärmemengenzähler", "U32", 1.0, "l/h", 1 },            // #51
  //{WP_REGISTER_VALUES_TEMP_VERDAMPF, "Verdampfungstemperatur", "F32", 0.1, "°C", 1 },                    // #52
  //{WP_REGISTER_VALUES_TEMP_ANSAUG_VD, "Ansaugtemparatur_VD", "F32", 0.1, "°C", 1 },                      // #53
  //{WP_REGISTER_VALUES_TEMP_VD_HEIZUNG, "Temperatur_VD_Heizung", "F32", 0.1, "°C", 1 },                   // #54
  //{WP_REGISTER_VALUES_TEMP_UEBERHITZ, "Ueberhitzung", "F32", 0.1, "K", 1 },                              // #55
  //{WP_REGISTER_VALUES_TEMP_UEBERHITZ_SOLL, "Ueberhitzung_Soll", "F32", 0.1, "K", 1 },                    // #56
  //{WP_REGISTER_VALUES_DRUCK_HD, "Hochdruckpressostat", "F32", 0.01, "bar", 1 },                          // #57
  //{WP_REGISTER_VALUES_DRUCK_ND, "Niederdruckpressostat", "F32", 0.01, "bar", 1 },                        // #58
  //{WP_REGISTER_VALUES_IO_VD_HEIZUNG, "VD_Heizung_I/O", "I/O", 1.0, "", 1 },                              // #59
  {WP_REGISTER_VALUES_RPM_UMWAELZPUMPE, "Drehzahl_Umwälzpumpe", "U32", 1.0, "rpm", 1 },                  // #60
  {WP_REGISTER_VALUES_RPM_VENTI, "Drehzahl_Ventilator", "U32", 1.0, "rpm", 1 },                          // #61
  {WP_REGISTER_VALUES_TEMP_VL_SOLL, "Temperatur_Vorlauf_Soll", "F32", 0.1, "°C", 1 },                    // #62
  {WP_REGISTER_VALUES_RPM_VD1, "Drehzahl_Verdichter", "U32", 1.0, "rpm", 1 },                            // #63
  {WP_REGISTER_VALUES_RPM_VD1_SOLL, "Drehzahl_Verdichter_Soll", "U32", 1.0, "rpm", 1 },                  // #64
  //{WP_REGISTER_VALUES_TEMP_FL1, "Temperatur_flüssiges_Kältemittel_Heizung", "F32", 0.1, "°C", 1 },       // #65
  //{WP_REGISTER_VALUES_TEMP_FL2, "Temperatur_flüssiges_Kältemittel_Kühlung", "F32", 0.1, "°C", 1 },       // #66
  //{WP_REGISTER_VALUES_DURCHFLUSS, "Durchfluss", "U32", 1.0, "l/h", 1 },                                  // #67
  {WP_REGISTER_VALUES_LEISTUNG_HEIZUNG, "Leistung_Heizung", "F32", 0.001, "kW", 1 },                     // #68
  //{WP_REGISTER_VALUES_FREIGABE_KUEHLUNG, "Freigabe_Kühlung", "F32", 0.000277778, "h", 1 }                // #69
};

struct wpRegisterStateStruct {
  uint16_t wpRegisterStateID;
  char *wpRegisterStateDescriptor;
};

static struct wpRegisterStateStruct wpRegisterSysValuesState[] = {
  { WP_STATE_HEIZEN, "Heizen" },
  { WP_STATE_WARMWASSER, "Warmwasser" },
  { WP_STATE_AUSHEIZPROGRAMM, "Ausheizprogramm" },
  { WP_STATE_ABTAU, "EVU" },
  { WP_STATE_EVU, "Abtau" },
  { WP_STATE_PUMPENVORLAUF, "Pumpenvorlauf" },
  { WP_STATE_FEHLER, "Fehler" },
  { WP_STATE_OFFLINE, "Offline" }
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void esp32Restart();

void serialOTAReceiver(uint8_t *data, size_t length);

unsigned int tcpTransmitBuffer(uint8_t *tcpTxBuffer);

void outputInfoline();

void outputWPData(uint8_t *tcpBuffer, uint16_t tcpBufferIndex, char *wpEntityDesriptor, char *wpEntityDataType, float wpEntityDataFactor, char *wpEntityUnit, bool mqttActive, bool mqttRetained, uint16_t mqttQoS);

bool mqttPublishTopicPayloadUInt32(String mqttTopicID, String mqttTopicEntity, uint32_t mqttPayloadUInt32, bool mqttRetained, uint16_t mqttQoS);
bool mqttPublishTopicPayloadFloat32(String mqttTopicID, String mqttTopicEntity, float mqttPayloadFloat32, bool mqttRetained, uint16_t mqttQoS);

//EOF