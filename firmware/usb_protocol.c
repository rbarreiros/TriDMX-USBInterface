
#include <ch.h>
#include <hal.h>
#include "usbdrv.h"
#include "usb_protocol.h"
#include "dmx.h"

uint8_t usbProtoReadCmd(BaseChannel *chn)
{
  uint8_t cmd[2];
  uint8_t data[USB_MAX_PACKET_SIZE];
  uint8_t err = MASK_REPLY_ERR;

  if(chnReadTimeout(chn, cmd, 2, MS2ST(25)) < 2)
  {
    chnPutTimeout(chn, err | MASK_ERR_WRONG_HEADER, MS2ST(25));
    return MASK_ERR_WRONG_HEADER;
  }

  if((uint8_t)chnReadTimeout(chn, data, cmd[1] - 2, MS2ST(25)) < (cmd[1] - 2))
  {
    chnPutTimeout(chn, err | MASK_ERR_WRONG_DATA_SIZE, MS2ST(25));
    return MASK_ERR_WRONG_DATA_SIZE;
  }

  uint8_t status = 0;
  switch(cmd[0])
  {
    case CMD_DMX_OUT_UPDATE:
      status = dmxUpdate(data, cmd[1] - 2);
      if(status != 0) chnPutTimeout(chn, err | MASK_ERR_DMX_OUT_UPDATE, MS2ST(25));
      break;

    case CMD_DMX_OUT_STREAM:
    case CMD_DMX_OUT_STRCONT:
      status = dmxSetStream(data, cmd[1] - 2, (cmd[0] == CMD_DMX_OUT_STREAM) ? 1 : 0);
      if(status != 0) chnPutTimeout(chn, err | MASK_ERR_DMX_OUT_STREAM, MS2ST(25));
      //chnPutTimeout(chn, status, MS2ST(25));
      break;

    case CMD_NOP:
      break;
  }

  chnPutTimeout(chn, MASK_REPLY_OK, MS2ST(25));

  return 0;
}
