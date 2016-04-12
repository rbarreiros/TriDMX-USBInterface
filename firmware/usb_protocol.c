
#include <ch.h>
#include <hal.h>
#include "config.h"
#include "usbdrv.h"
#include "usb_protocol.h"
#include "dmx.h"

uint8_t usbProtoReadCmd(BaseChannel *chn)
{
  cmd_header_t *header;
  uint8_t data[USB_MAX_PACKET_SIZE];
  uint8_t err = MASK_REPLY_ERR;
  uint8_t ret = MASK_REPLY_OK;
  uint8_t data_read = 0;

  while( (data_read = chnReadTimeout(chn, data, USB_MAX_PACKET_SIZE, TIME_IMMEDIATE)) != 0 )
  {
    header = (cmd_header_t*)data;

    if(header->len > (USB_MAX_PACKET_SIZE - 3))
    {
      ret = err | MASK_ERR_DATA_TOOBIG;
      continue;
    }

    if(data_read > header->len + 3)
    {
      ret = err | MASK_ERR_DATA_TOOBIG;
      continue;
    }

    uint8_t status = 0;
    DeviceConfig dev_cfg;
    switch(header->cmd)
    {
      // TODO
      case CMD_SET_MODE:
        ret = MASK_REPLY_OK;
        break;

        // TODO
      case CMD_GET_MODE:
        chnWriteTimeout(chn, (uint8_t*)&dev_cfg, sizeof(dev_cfg), MS2ST(25));
        ret = MASK_REPLY_OK;
        break;
        
      case CMD_DMX_OUT_STREAM:
      case CMD_DMX_OUT_STRCONT:
        status = dmxSetStream(header->port, &data[3], header->len, (header->cmd == CMD_DMX_OUT_STREAM) ? 1 : 0);
        if(status != 0) ret = err | MASK_ERR_DMX_OUT_STREAM;        
        break;

      case CMD_DMX_OUT_UPDATE:
        status = dmxUpdate(header->port, &data[3], header->len);
        if(status != 0) ret = err | MASK_ERR_DMX_OUT_UPDATE;
        break;

        // TODO
      case CMD_DMX_IN_STREAM:
      case CMD_DMX_IN_STRCONT:
        ret = MASK_REPLY_OK;
        break;

        // TODO
      case CMD_DMX_IN_UPDATE:
        ret = MASK_REPLY_OK;
        break;

      case CMD_NOP:
        ret = MASK_REPLY_OK;
        break;
        
      default:
        ret = err | MASK_ERR_NOT_IMPLEMENTED;
    }
  }
  
  return ret;
}
