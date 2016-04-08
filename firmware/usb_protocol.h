#ifndef __USB_PROTOCOL_H__
#define __USB_PROTOCOL_H__

typedef struct
{
  uint8_t cmd;
  uint8_t len;
  uint8_t port;
  uint8_t *data;
} cmd_header_t;

// Port Modes

#define MODE_USB_DIR  0x01 // DMX Direction, 0 out 1 in
#define MODE_MIRROR_1 0x02 // Port mirrors port 1 (not applicable to port 1)
#define MODE_MIRROR_2 0x03 // Port mirrors port 2 (not applicable to port 2)
#define MODE_MIRROR_3 0x04 // Port mirrors port 3 (not applicable to port 3)
#define MODE_MERGE_23 0x05 // Merges Port 2 and 3 outputs port 1 (makes 2 and 3 inputs, 1 output)
#define MODE_MERGE_13 0x06 // Merges Port 1 and 3 outputs port 2 (makes 1 and 3 inputs, 2 output)
#define MODE_MERGE_12 0x07 // Merges Port 1 and 2 outputs port 3 (makes 1 and 2 inputs, 3 output)

// Commands

#define CMD_NOP 0x00

#define CMD_SET_MODE        0x01 // [CMD] [PORT] [MODE]
#define CMD_GET_MODE        0x02 // [CMD] [PORT] returns DMXPortConfig struct

#define CMD_DMX_OUT_UPDATE  0x10 // [CMD] [LEN] [PORT] [CH] [VAL] ... [CH] [VAL] (for low channel count updates)
#define CMD_DMX_OUT_STREAM  0x11 // [CMD] [LEN] [PORT] [CH1_VAL] [CH2_VAL] [CH3_VAL]  (First 512 channels)
#define CMD_DMX_OUT_STRCONT 0x12 // [CMD] [LEN] [PORT] [CHN_VAL] ... (remaining 512 channels)

#define CMD_DMX_IN_UPDATE  0x13
#define CMD_DMX_IN_STREAM  0x14
#define CMD_DMX_IN_STRCONT 0x15

#define CMD_BOOTLOADER 0xfe // Reset to bootloader for firmware update

// Error

#define MASK_REPLY_OK   0x00
#define MASK_REPLY_INFO 0x40
#define MASK_REPLY_ERR  0x80


#define MASK_ERR_WRONG_HEADER    0x01
#define MASK_ERR_WRONG_COMMAND   0x02
#define MASK_ERR_WRONG_DATA_SIZE 0x03
#define MASK_ERR_DATA_TOOBIG     0x04

#define MASK_ERR_DMX_OUT_UPDATE  0x08
#define MASK_ERR_DMX_OUT_STREAM  0x09
#define MASK_ERR_DMX_OUT_STRCONT 0x0A

#define MASK_ERR_NOT_IMPLEMENTED 0x0F

uint8_t usbProtoReadCmd(BaseChannel *chn);

#endif
