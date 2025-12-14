/*
  ESP32_WP_CTA_Aeroheat_CI_8is_MQTT.h
*/

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// esp32 restart info
#define ESP32_RESTART_SYSTEM               0
#define ESP32_RESTART_WIFI_CONNECTION      1
#define ESP32_RESTART_MQTT_CONNECTION      2
#define ESP32_RESTART_WP_TCP_CONNECTION    3
#define ESP32_RESTART_WP_TCP_COMMUNICATION 4
#define ESP32_RESTART_OTA_RESTART          5
#define ESP32_RESTART_OTA_HALT             6

#define ESP32_RESTART_INFO_ELEMENTS 7

struct esp32RestartCounterStruct {
  uint8_t esp32RestartID;
  char *esp32RestartReasonNVS;  // max. 15 characters!
  char *esp32RestartReason;
  uint32_t esp32RestartCounter;
};

static struct esp32RestartCounterStruct esp32RestartInfo[] = {
  { ESP32_RESTART_SYSTEM, "SYSTEM", "System", 0},
  { ESP32_RESTART_WIFI_CONNECTION, "WIFI", "WiFi Connection Error", 0},
  { ESP32_RESTART_MQTT_CONNECTION, "MQTT", "MQTT Connection Error", 0},
  { ESP32_RESTART_WP_TCP_CONNECTION, "WP_TCP", "WP TCP Connection Error", 0},
  { ESP32_RESTART_WP_TCP_COMMUNICATION, "WP_TCP_CRC", "WP TCP Communication Error", 0},
  { ESP32_RESTART_OTA_RESTART, "OTA_RESTART", "OTA Restart", 0},
  { ESP32_RESTART_OTA_HALT, "OTA_HALT", "OTA Halt", 0}
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// wp value #137 / register block SysTime
#define WP_REGISTER_VALUES_SYSTIME_TIMESTAMP_AKT  137  // #0 [548]

// wp values / register block #0
#define WP_REGISTER_VALUES_NUMBER_BYTES        1108
#define WP_REGISTER_VALUES_HEAD_ELEMENT        13
#define WP_REGISTER_VALUES_ELEMENTS            40   // active elements
#define WP_REGISTER_VALUES_ACTIVE              1
#define WP_REGISTER_VALUES_TEMP_VL             13   // #0 [52]
#define WP_REGISTER_VALUES_TEMP_RL             14   // #1 [56]
#define WP_REGISTER_VALUES_TEMP_RL_SOLL        15   // #2 [60]
#define WP_REGISTER_VALUES_TEMP_RL_EXT         16   // #3 [64]
#define WP_REGISTER_VALUES_TEMP_HG             17   // #5 [68]
#define WP_REGISTER_VALUES_TEMP_AUSSEN         18   // #5 [72]
#define WP_REGISTER_VALUES_TEMP_MITTEL         19   // #6 [76]
#define WP_REGISTER_VALUES_TEMP_WQ             22   // #7 [88]
#define WP_REGISTER_VALUES_IO_ASD              32   // #8 [128]
#define WP_REGISTER_VALUES_IO_EVU              34   // #9 [136]
#define WP_REGISTER_VALUES_IO_HD               35   // #10 [140]
#define WP_REGISTER_VALUES_IO_MOTOR            36   // #11 [144]
#define WP_REGISTER_VALUES_IO_ND               37   // #12 [148]
#define WP_REGISTER_VALUES_IO_PEX              38   // #13 [152]
#define WP_REGISTER_VALUES_IO_SWT              39   // #14 [156]
#define WP_REGISTER_VALUES_IO_ABTAUVENTIL      40   // #15 [160]
#define WP_REGISTER_VALUES_IO_BUP              41   // #16 [164]
#define WP_REGISTER_VALUES_IO_HUB              42   // #17 [168]
#define WP_REGISTER_VALUES_IO_VENTILATOR       45   // #18 [180]
#define WP_REGISTER_VALUES_IO_VENTILATOR_BOSUP 46   // #19 [184]
#define WP_REGISTER_VALUES_IO_VD1              47   // #20 [188]
#define WP_REGISTER_VALUES_IO_ZIP              49   // #21 [196]
#define WP_REGISTER_VALUES_IP_ZUP              50   // #22 [200]
#define WP_REGISTER_VALUES_IO_ZWE1             51   // #23 [204]
#define WP_REGISTER_VALUES_IO_FUP2             54   // #24 [216]
#define WP_REGISTER_VALUES_IO_SLP              55   // #25 [220]
#define WP_REGISTER_VALUES_IO_SUP              56   // #26 [224]
#define WP_REGISTER_VALUES_HH_VD1              59   // #27 [236]
#define WP_REGISTER_VALUES_IMPULS_VD1          60   // #28 [240]
#define WP_REGISTER_VALUES_HH_WP               66   // #29 [264]
#define WP_REGISTER_VALUES_HH_HEIZUNG          67   // #30 [268]
#define WP_REGISTER_VALUES_HH_WP_EIN           70   // #31 [280]
#define WP_REGISTER_VALUES_WP_TYP              81   // #32 [324]
#define WP_REGISTER_VALUES_STAT_BV             82   // #33 [328]
#define WP_REGISTER_VALUES_STATE_WP            83   // #34 [332]
#define WP_REGISTER_VALUES_IP_ADDR             94   // #35 [376]
#define WP_REGISTER_VALUES_IP_NET_MASK         95   // #36 [380]
#define WP_REGISTER_VALUES_IP_BROADCAST_ADDR   96   // #37 [384]
#define WP_REGISTER_VALUES_IP_GATEWAY_ADDR     97   // #38 [388]
#define WP_REGISTER_VALUES_ERR_TIMESTAMP0      98   // #39 [392]
#define WP_REGISTER_VALUES_ERR_TIMESTAMP1      99   // #40 [396]
#define WP_REGISTER_VALUES_ERR_TIMESTAMP2      100  // #41 [400]
#define WP_REGISTER_VALUES_ERR_TIMESTAMP3      101  // #42 [404]
#define WP_REGISTER_VALUES_ERR_TIMESTAMP4      102  // #43 [408]
#define WP_REGISTER_VALUES_ERR_CODE0           103  // #44 [412]
#define WP_REGISTER_VALUES_ERR_CODE1           104  // #45 [416]
#define WP_REGISTER_VALUES_ERR_CODE2           105  // #46 [420]
#define WP_REGISTER_VALUES_ERR_CODE3           106  // #47 [424]
#define WP_REGISTER_VALUES_ERR_CODE4           107  // #48 [428]
#define WP_REGISTER_VALUES_ERR_COUNT           108  // #49 [432]
#define WP_REGISTER_VALUES_SHUTDOWN_REASON0    109  // #50 [436]
#define WP_REGISTER_VALUES_SHUTDOWN_REASON1    110  // #51 [440]
#define WP_REGISTER_VALUES_SHUTDOWN_REASON2    111  // #52 [444]
#define WP_REGISTER_VALUES_SHUTDOWN_REASON3    112  // #53 [448]
#define WP_REGISTER_VALUES_SHUTDOWN_REASON4    113  // #54 [452]
#define WP_REGISTER_VALUES_SHUTDOWN_TIMESTAMP0 114  // #55 [452]
#define WP_REGISTER_VALUES_SHUTDOWN_TIMESTAMP1 115  // #56 [452]
#define WP_REGISTER_VALUES_SHUTDOWN_TIMESTAMP2 116  // #57 [452]
#define WP_REGISTER_VALUES_SHUTDOWN_TIMESTAMP3 117  // #58 [452]
#define WP_REGISTER_VALUES_SHUTDOWN_TIMESTAMP4 118  // #59 [452]
#define WP_REGISTER_VALUES_COMFORT_PLATINE     119  // #60 [456]
#define WP_REGISTER_VALUES_HH_ABTAUEN          144  // #61 [576]
#define WP_REGISTER_VALUES_ENERGIE_WP          154  // #62 [616]
#define WP_REGISTER_VALUES_ENERGIE_RESET       157  // #63 [628]
#define WP_REGISTER_VALUES_IO_OUT1             159  // #64 [636]
#define WP_REGISTER_VALUES_IO_OUT2             160  // #65 [640]
#define WP_REGISTER_VALUES_DURCHFLUSS_WMZ      176  // #66 [704]
#define WP_REGISTER_VALUES_TEMP_VERDAMPF       178  // #67 [712]
#define WP_REGISTER_VALUES_TEMP_ANSAUG_VD      179  // #68 [716]
#define WP_REGISTER_VALUES_TEMP_VD_HEIZUNG     180  // #69 [720]
#define WP_REGISTER_VALUES_TEMP_UEBERHITZ      181  // #70 [724]
#define WP_REGISTER_VALUES_TEMP_UEBERHITZ_SOLL 182  // #71 [728]
#define WP_REGISTER_VALUES_DRUCK_HD            183  // #72 [732]
#define WP_REGISTER_VALUES_DRUCK_ND            184  // #73 [736]
#define WP_REGISTER_VALUES_IO_VD_HEIZUNG       185  // #74 [740]
#define WP_REGISTER_VALUES_RPM_UMWAELZPUMPE    186  // #75 [744]
#define WP_REGISTER_VALUES_RPM_VENTI           187  // #76 [748]
#define WP_REGISTER_VALUES_TEMP_VL_SOLL        192  // #77 [768]
#define WP_REGISTER_VALUES_RPM_VD1             234  // #78 [936]
#define WP_REGISTER_VALUES_RPM_VD1_SOLL        239  // #79 [956]
#define WP_REGISTER_VALUES_TEMP_FL1            252  // #80 [1008]
#define WP_REGISTER_VALUES_TEMP_FL2            253  // #81 [1012]
#define WP_REGISTER_VALUES_DURCHFLUSS          257  // #82 [1028]
#define WP_REGISTER_VALUES_LEISTUNG_HEIZUNG    260  // #83 [1040]
#define WP_REGISTER_VALUES_FREIGABE_KUEHLUNG   263  // #84 [1052]

#define WP_REGISTER_BLOCK_COUNT 1  // number of wp register blocks

// wp state code (WP_REGISTER_VALUES_STATE_WP)
#define WP_STATE_HEIZEN          0x00
#define WP_STATE_WARMWASSER      0x01
#define WP_STATE_AUSHEIZPROGRAMM 0x02
#define WP_STATE_EVU             0x03
#define WP_STATE_ABTAUEN         0x04
#define WP_STATE_PUMPENVORLAUF   0x05
#define WP_STATE_HEIZEN_EXTERN   0x06
#define WP_STATE_OFFLINE         0x07

// wp shutdown code (WP_REGISTER_VALUES_SHUTDOWN_REASON)
#define WP_SHUTDOWN_REASON_STOERUNG_WP               0x01
#define WP_SHUTDOWN_REASON_STOERUNG_ANLAGE           0x02
#define WP_SHUTDOWN_REASON_ZWEITER_WAERMEERZEUGER    0x03
#define WP_SHUTDOWN_REASON_EVU_SPERRE                0x04
#define WP_SHUTDOWN_REASON_LUFTABTAU                 0x05
#define WP_SHUTDOWN_REASON_TEMP_EINSATZGRENZE_MAX    0x06
#define WP_SHUTDOWN_REASON_TEMP_EINSATZGRENZE_MIN    0x07
#define WP_SHUTDOWN_REASON_TEMP_UNTERE_EINSATZGRENZE 0x08
#define WP_SHUTDOWN_REASON_ANFORDERUNG               0x09

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
  //{WP_REGISTER_VALUES_TEMP_RL_EXT, "Temperatur_externer_Rücklauffühler", "F32", 0.1, "°C", 0 },          // #3
  //{WP_REGISTER_VALUES_TEMP_HG, "Temperatur_Heissgas", "F32", 0.1, "°C", 1 },                             // #4
  {WP_REGISTER_VALUES_TEMP_AUSSEN, "Temperatur_Aussenfühler", "F32", 0.1, "°C", 1 },                     // #5
  {WP_REGISTER_VALUES_TEMP_MITTEL, "Temperatur_Aussenfühler_Durchschnitt_24h", "F32", 0.1, "°C", 1 },    // #6
  //{WP_REGISTER_VALUES_TEMP_WQ, "Temperatur_Wärmequellenfühler_Eintritt", "F32", 0.1, "°C", 1 },          // #7
  //{WP_REGISTER_VALUES_IO_ASD, "ASD_I/O", "I/O", 1.0, "", 0 },                                            // #8
  //{WP_REGISTER_VALUES_IO_EVU, "EVU_I/O", "I/O", 1.0, "", 0 },                                            // #9
  //{WP_REGISTER_VALUES_IO_HD, "HD_I/O", "I/O", 1.0, "", 0 },                                              // #10
  //{WP_REGISTER_VALUES_IO_MOTOR, "Motor_I/O", "I/O", 1.0, "", 0 },                                        // #11
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
  //{WP_REGISTER_VALUES_HH_VD1, "Betriebsstunden_Verdichter", "F32", 0.000277777777777778, "h", 1 }        // #27
  {WP_REGISTER_VALUES_IMPULS_VD1, "Impulse_Verdichter", "U32", 1.0, "", 1 },                             // #28
  {WP_REGISTER_VALUES_HH_WP, "Betriebsstunden", "F32", 0.000277777777777778, "h", 1 },                   // #29
  //{WP_REGISTER_VALUES_HH_HEIZUNG, "Betriebsstunden_Heizung", "F32", 0.000277777777777778, "h", 1 },      // #30
  {WP_REGISTER_VALUES_HH_WP_EIN, "Einschaltdauer", "F32", 0.000277777777777778, "h", 1 },                // #31 
  {WP_REGISTER_VALUES_WP_TYP, "Wärmepumpen_Typ", "U32", 1.0, "", 0 },                                    // #32
  //{WP_REGISTER_VALUES_STAT_BV, "Bivalenz_Stufe", "Status", 1.0, "", 1 },                                 // #33
  {WP_REGISTER_VALUES_STATE_WP, "Betriebszustand", "Status", 1.0, "", 1 },                               // #34
  //{WP_REGISTER_VALUES_IP_ADDR, "IP_Address", "IP", 1.0, "", 0 },                                         // #35
  //{WP_REGISTER_VALUES_IP_NET_MASK, "Subnet_ Mask", "IP", 1.0, "", 0 },                                   // #36
  //{WP_REGISTER_VALUES_IP_BROADCAST_ADDR, "Broadcast_Address", "IP", 1.0, "", 0 },                        // #37
  //{WP_REGISTER_VALUES_IP_GATEWAY_ADDR, "Gateway_Address", "IP", 1.0, "", 0 },                            // #38
  {WP_REGISTER_VALUES_ERR_CODE0, "Error_Code0", "U32", 1.0, "", 1 },                                     // #39
  {WP_REGISTER_VALUES_ERR_TIMESTAMP0, "Error_Timestamp0", "UX", 1.0, "", 1 },                            // #40
  {WP_REGISTER_VALUES_ERR_CODE1, "Error_Code1", "U32", 1.0, "", 1 },                                     // #41
  {WP_REGISTER_VALUES_ERR_TIMESTAMP1, "Error_Timestamp1", "UX", 1.0, "", 1 },                            // #42
  {WP_REGISTER_VALUES_ERR_CODE2, "Error_Code2", "U32", 1.0, "", 1 },                                     // #43
  {WP_REGISTER_VALUES_ERR_TIMESTAMP2, "Error_Timestamp2", "UX", 1.0, "", 1 },                            // #44
  {WP_REGISTER_VALUES_ERR_CODE3, "Error_Code3", "U32", 1.0, "", 1 },                                     // #45
  {WP_REGISTER_VALUES_ERR_TIMESTAMP3, "Error_Timestamp3", "UX", 1.0, "", 1 },                            // #46
  {WP_REGISTER_VALUES_ERR_CODE4, "Error_Code4", "U32", 1.0, "", 1 },                                     // #47
  {WP_REGISTER_VALUES_ERR_TIMESTAMP4, "Error_Timestamp4", "UX", 1.0, "", 1 },                            // #48
  //{WP_REGISTER_VALUES_ERR_COUNT, "Error_Count", "U32", 1.0, "", 1 },                                     // #49
  {WP_REGISTER_VALUES_SHUTDOWN_REASON0, "Abschalt_Ursache0", "Status", 1.0, "", 1 },                     // #50
  {WP_REGISTER_VALUES_SHUTDOWN_TIMESTAMP0, "Abschalt_Timestamp0", "UX", 1.0, "", 1 },                    // #51
  {WP_REGISTER_VALUES_SHUTDOWN_REASON1, "Abschalt_Ursache1", "Status", 1.0, "", 1 },                     // #52
  {WP_REGISTER_VALUES_SHUTDOWN_TIMESTAMP1, "Abschalt_Timestamp1", "UX", 1.0, "", 1 },                    // #53
  {WP_REGISTER_VALUES_SHUTDOWN_REASON2, "Abschalt_Ursache2", "Status", 1.0, "", 1 },                     // #54
  {WP_REGISTER_VALUES_SHUTDOWN_TIMESTAMP2, "Abschalt_Timestamp2", "UX", 1.0, "", 1 },                    // #55
  {WP_REGISTER_VALUES_SHUTDOWN_REASON3, "Abschalt_Ursache3", "Status", 1.0, "", 1 },                     // #56
  {WP_REGISTER_VALUES_SHUTDOWN_TIMESTAMP3, "Abschalt_Timestamp3", "UX", 1.0, "", 1 },                    // #57
  {WP_REGISTER_VALUES_SHUTDOWN_REASON4, "Abschalt_Ursache4", "Status", 1.0, "", 1 },                     // #58
  {WP_REGISTER_VALUES_SHUTDOWN_TIMESTAMP4, "Abschalt_Timestamp4", "UX", 1.0, "", 1 },                    // #59
  //{WP_REGISTER_VALUES_COMFORT_PLATINE, "Komfort_Platine", "U32", 1.0, "", 0 },                           // #60
  {WP_REGISTER_VALUES_HH_ABTAUEN, "Abtauen", "F32", 0.000277777777777778, "h", 1 },                      // #61
  {WP_REGISTER_VALUES_ENERGIE_WP, "Wärmemenge_Heizung", "F32", 0.1, "kWh", 1 },                          // #62
  //{WP_REGISTER_VALUES_ENERGIE_RESET, "Wärmemenge_seit_Reset", "F32", 0.1, "kWh", 0 },                    // #63
  //{WP_REGISTER_VALUES_IO_OUT1, "Analog_Out1", "F32", 0.01, "V", 0 },                                     // #64
  //{WP_REGISTER_VALUES_IO_OUT2, "Analog_Out2", "F32", 0.01, "V", 0 },                                     // #65
  {WP_REGISTER_VALUES_DURCHFLUSS_WMZ, "Durchfluss_Wärmemengenzähler", "U32", 1.0, "l/h", 1 },            // #66
  //{WP_REGISTER_VALUES_TEMP_VERDAMPF, "Verdampfungstemperatur", "F32", 0.1, "°C", 1 },                    // #67
  //{WP_REGISTER_VALUES_TEMP_ANSAUG_VD, "Ansaugtemparatur_VD", "F32", 0.1, "°C", 1 },                      // #68
  //{WP_REGISTER_VALUES_TEMP_VD_HEIZUNG, "Temperatur_VD_Heizung", "F32", 0.1, "°C", 1 },                   // #69
  {WP_REGISTER_VALUES_TEMP_UEBERHITZ, "Temperatur_Überhitzung", "F32", 0.1, "K", 1 },                    // #70
  {WP_REGISTER_VALUES_TEMP_UEBERHITZ_SOLL, "Temperatur_Überhitzung_Soll", "F32", 0.1, "K", 1 },          // #71
  //{WP_REGISTER_VALUES_DRUCK_HD, "Hochdruckpressostat", "F32", 0.01, "bar", 1 },                          // #72
  //{WP_REGISTER_VALUES_DRUCK_ND, "Niederdruckpressostat", "F32", 0.01, "bar", 1 },                        // #73
  //{WP_REGISTER_VALUES_IO_VD_HEIZUNG, "VD_Heizung_I/O", "I/O", 1.0, "", 1 },                              // #74
  {WP_REGISTER_VALUES_RPM_UMWAELZPUMPE, "Drehzahl_Umwälzpumpe", "U32", 1.0, "rpm", 1 },                  // #75
  {WP_REGISTER_VALUES_RPM_VENTI, "Drehzahl_Ventilator", "U32", 1.0, "rpm", 1 },                          // #76
  //{WP_REGISTER_VALUES_TEMP_VL_SOLL, "Temperatur_Vorlauf_Soll", "F32", 0.1, "°C", 1 },                    // #77
  {WP_REGISTER_VALUES_RPM_VD1, "Drehzahl_Verdichter", "U32", 1.0, "rpm", 1 },                            // #78
  {WP_REGISTER_VALUES_RPM_VD1_SOLL, "Drehzahl_Verdichter_Soll", "U32", 1.0, "rpm", 1 },                  // #79
  //{WP_REGISTER_VALUES_TEMP_FL1, "Temperatur_flüssiges_Kältemittel_Heizung", "F32", 0.1, "°C", 1 },       // #80
  //{WP_REGISTER_VALUES_TEMP_FL2, "Temperatur_flüssiges_Kältemittel_Kühlung", "F32", 0.1, "°C", 1 },       // #81
  //{WP_REGISTER_VALUES_DURCHFLUSS, "Durchfluss", "U32", 1.0, "l/h", 1 },                                  // #82
  {WP_REGISTER_VALUES_LEISTUNG_HEIZUNG, "Leistung_Heizung", "F32", 0.001, "kW", 1 },                     // #83
  //{WP_REGISTER_VALUES_FREIGABE_KUEHLUNG, "Freigabe_Kühlung", "F32", 0.000277777777777778, "h", 1 }       // #84
};

struct wpRegisterStateStruct {
  uint16_t wpRegisterStateID;
  char *wpRegisterStateDescriptor;
};

static struct wpRegisterStateStruct wpRegisterSysValuesState[] = {
  { WP_STATE_HEIZEN, "Heizen" },
  { WP_STATE_WARMWASSER, "Warmwasser" },
  { WP_STATE_AUSHEIZPROGRAMM, "Ausheizprogramm" },
  { WP_STATE_EVU, "EVU" },
  { WP_STATE_ABTAUEN, "Abtauen" },
  { WP_STATE_PUMPENVORLAUF, "Pumpenvorlauf" },
  { WP_STATE_HEIZEN_EXTERN, "Heizen (extern)" },
  { WP_STATE_OFFLINE, "Offline" }
};

struct wpRegisterShutdownReasonStruct {
  uint16_t wpRegisterShutdownReasonID;
  char *wpRegisterShutdownReasonDescriptor;
};

static struct wpRegisterShutdownReasonStruct wpRegisterSysValuesShutdownReason[] = {
  { WP_SHUTDOWN_REASON_STOERUNG_WP, "Störung Wärmepumpe" },
  { WP_SHUTDOWN_REASON_STOERUNG_ANLAGE, "Störung Anlage" },
  { WP_SHUTDOWN_REASON_ZWEITER_WAERMEERZEUGER, "Betriebsart zweiter Wärmeerzeuger" },
  { WP_SHUTDOWN_REASON_EVU_SPERRE, "EVU Sperre" },
  { WP_SHUTDOWN_REASON_LUFTABTAU, "Lauftabtau" },
  { WP_SHUTDOWN_REASON_TEMP_EINSATZGRENZE_MAX, "Temperatur Einsatzgrenze max." },
  { WP_SHUTDOWN_REASON_TEMP_EINSATZGRENZE_MIN, "Temperatur Einsatzgrenze min." },
  { WP_SHUTDOWN_REASON_TEMP_UNTERE_EINSATZGRENZE, "Untere Einsatzgrenze" },
  { WP_SHUTDOWN_REASON_ANFORDERUNG, "Anforderung" }
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void esp32Restart(uint8_t esp32RestartID);

void serialOTAReceiver(uint8_t *data, size_t length);

unsigned int tcpTransmitBuffer(uint8_t *tcpTxBuffer);

void outputInfoline();

void outputWPData(uint8_t *tcpBuffer, uint16_t tcpBufferIndex, char *wpEntityDescriptor, char *wpEntityDataType, float wpEntityDataFactor, char *wpEntityUnit, bool mqttActive, bool mqttRetained, uint16_t mqttQoS);

bool mqttPublishTopicPayloadString(String mqttTopicID, String mqttTopicEntity, String mqttPayloadString, bool mqttRetained, uint16_t mqttQoS);
bool mqttPublishTopicPayloadUInt32(String mqttTopicID, String mqttTopicEntity, uint32_t mqttPayloadUInt32, bool mqttRetained, uint16_t mqttQoS);
bool mqttPublishTopicPayloadFloat32(String mqttTopicID, String mqttTopicEntity, float mqttPayloadFloat32, bool mqttRetained, uint16_t mqttQoS);

//EOF