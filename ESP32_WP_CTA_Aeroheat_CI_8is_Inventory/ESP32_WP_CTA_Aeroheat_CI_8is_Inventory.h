/*
  ESP32_WP_CTA_Aeroheat_CI_8is_Inventory.h
*/

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// wp register blocks
#define WP_REGISTER_BLOCK_VALUES 0

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// wp values [13-265] / register block #0
#define WP_REGISTER_VALUES_NUMBER_BYTES 1064
#define WP_REGISTER_VALUES_HEAD_ELEMENT 0
#define WP_REGISTER_VALUES_TAIL_ELEMENT ((WP_REGISTER_VALUES_NUMBER_BYTES / 4) - 1)  // = 265
#define WP_REGISTER_VALUES_ELEMENTS     (WP_REGISTER_VALUES_TAIL_ELEMENT - WP_REGISTER_VALUES_HEAD_ELEMENT + 1)  // = 266
#define WP_REGISTER_VALUES_ACTIVE       1

#define WP_REGISTER_BLOCK_COUNT 1  // number of wp register blocks

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
struct wpRegisterBlockStruct {
  uint16_t wpRegisterBlockHeadElement;
  uint16_t wpRegisterBlockTailElement;
  uint16_t wpRegisterBlockElements;
  uint16_t wpRegisterBlockActive;
};

static struct wpRegisterBlockStruct wpRegisterBlocks[] = {
  { WP_REGISTER_VALUES_HEAD_ELEMENT, WP_REGISTER_VALUES_TAIL_ELEMENT, WP_REGISTER_VALUES_ELEMENTS, WP_REGISTER_VALUES_ACTIVE }  // #0
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void esp32Restart();

void serialOTAReceiver(uint8_t *data, size_t length);

unsigned int tcpTransmitBuffer(uint8_t *tcpTxBuffer);

//EOF