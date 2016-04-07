
#include <ch.h>
#include <hal.h>
#include "usbdrv.h"
#include "usb_protocol.h"
#include "dmx.h"

uint8_t usbProtoReadCmd(BaseChannel *chn)
{
  cmd_header_t header;
  uint8_t data[USB_MAX_PACKET_SIZE];
  uint8_t err = MASK_REPLY_ERR;
  uint8_t data_read;

  if(chnReadTimeout(chn, (uint8_t *)&header, sizeof(cmd_header_t), TIME_IMMEDIATE) < sizeof(cmd_header_t))
    return err | MASK_ERR_WRONG_HEADER;

  if(header.len > USB_MAX_PACKET_SIZE)
    return err | MASK_ERR_DATA_TOOBIG;

  data_read = (uint8_t)chnReadTimeout(chn, data, (header.len - sizeof(header)), TIME_IMMEDIATE);
  if(data_read < (header.len - sizeof(header)))
    return err | MASK_ERR_WRONG_DATA_SIZE;

  uint8_t status = 0;
  switch(header.cmd)
  {
    case CMD_DMX_OUT_UPDATE:
      //status = dmxUpdate(data, cmd[1] - 2);
      //if(status != 0) chnPutTimeout(chn, err | MASK_ERR_DMX_OUT_UPDATE, MS2ST(25));
      break;

    case CMD_DMX_OUT_STREAM:
    case CMD_DMX_OUT_STRCONT:
      status = dmxSetStream(header.port, data, (header.len - sizeof(header)), (header.cmd == CMD_DMX_OUT_STREAM) ? 1 : 0);
      if(status != 0) return err | MASK_ERR_DMX_OUT_STREAM;
      break;

    case CMD_NOP:
      break;
    default:
      break;
  }
  
  return MASK_REPLY_OK;
}
