
#include <ch.h>
#include <hal.h>
#include "config.h"
#include "usbdrv.h"
#include "usb_protocol.h"
#include "dmx.h"

bool gDoShutdown = false;

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
    DMXPortConfig port_cfg;
    bool set;
    switch(header->cmd)
    {
      // TODO
      case CMD_SET_MODE:
        set = configSetPortConfig(header->port, (DMXPortConfig*)&data[3]);
        chnWriteTimeout(chn, (uint8_t*)&set, 1, MS2ST(25));
        ret = MASK_REPLY_OK;
        break;

        // TODO
      case CMD_GET_MODE:
        port_cfg = configGetPortConfig(header->port);
        chnWriteTimeout(chn, (uint8_t *)&port_cfg, sizeof(DMXPortConfig), MS2ST(25));
        ret = MASK_REPLY_OK;
        break;

      case CMD_PORT_ID:
        dmxIdentify(header->port);
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

      case CMD_BOOTLOADER:
        // Go into bootloader
        gDoShutdown = true;
        ret = MASK_REPLY_OK;
        break;
        
      default:
        ret = err | MASK_ERR_NOT_IMPLEMENTED;
    }
  }
  
  return ret;
}

// USB Thread
THD_WORKING_AREA(waUSB, USB_THREAD_SIZE);
THD_FUNCTION(USBThread, arg)
{
  event_listener_t el1;
  eventflags_t flags;
  
  chRegSetThreadName("USB");
  (void)arg;

  sduObjectInit(&SDU1);
  sduStart(&SDU1, &serusbcfg);

  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1500);
  usbStart(serusbcfg.usbp, &usbcfg);
  usbConnectBus(serusbcfg.usbp);

  chEvtRegisterMask(chnGetEventSource(&SDU1), &el1, ALL_EVENTS);

  while(USBD1.state != USB_READY) chThdSleepMilliseconds(10);
  while(SDU1.state != SDU_READY) chThdSleepMilliseconds(10);

  while(!chThdShouldTerminateX())
  {
    // Needs to timeout or else it hangs waiting for an
    // event when we wish to terminate the thread!
    chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(100));
    chSysLock();
    flags = chEvtGetAndClearFlagsI(&el1);
    chSysUnlock();

    if (flags & CHN_INPUT_AVAILABLE)
      usbProtoReadCmd((BaseChannel *)&SDU1);
  }

  // Stop USB here
  usbStop(serusbcfg.usbp);
  usbDisconnectBus(serusbcfg.usbp);
}
