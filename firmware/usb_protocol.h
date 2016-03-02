#ifndef __USB_PROTOCOL_H__
#define __USB_PROTOCOL_H__

// Commands

#define CMD_NOP 0x00

#define CMD_DMX_OUT_UPDATE  0x10 // [CMD] [LEN] [PORT] [CH] [VAL] ... [CH] [VAL] (for low channel count updates)
#define CMD_DMX_OUT_STREAM  0x11 // [CMD] [LEN] [PORT] [CH1_VAL] [CH2_VAL] [CH3_VAL]  (First 512 channels)
#define DMX_DMX_OUT_STRCONT 0x12 // [CMD] [LEN] [PORT] [CHN_VAL] ... (remaining 512 channels)

#define CMD_DMX_IN_UPDATE  0x13
#define CMD_DMX_IN_STREAM  0x14
#define CMD_DMX_IN_STRCONT 0x15

#define CMD_BOOTLOADER 0xfe // Reset to bootloader for firmware update

// Error

#define MASK_REPLY_OK  0x00
#define MASK_REPLY_ERR 0x80

#define MASK_ERR_WRONG_HEADER    0x01
#define MASK_ERR_WRONG_COMMAND   0x02
#define MASK_ERR_WRONG_DATA_SIZE 0x03

#define MASK_ERR_DMX_OUT_UPDATE  0x08
#define MASK_ERR_DMX_OUT_STREAM  0x09
#define MASK_ERR_DMX_OUT_STRCONT 0x0A

uint8_t usbProtoReadCmd(BaseChannel *chn);

#endif
