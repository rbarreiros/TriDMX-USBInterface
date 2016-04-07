
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ch.h"
#include "hal.h"
#include "usbdrv.h"
#include "usb_protocol.h"
#include "dmx.h"

// USB Thread
static THD_WORKING_AREA(waThread2, 8192);
static THD_FUNCTION(Thread2, arg)
{
  event_listener_t el1;
  eventflags_t flags;
  uint8_t clear_data[USB_MAX_PACKET_SIZE];
  
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

  uint8_t ret;
  while(!chThdShouldTerminateX())
  {
    chEvtWaitAny(ALL_EVENTS);
    chSysLock();
    flags = chEvtGetAndClearFlagsI(&el1);
    chSysUnlock();

    if (flags & CHN_INPUT_AVAILABLE)
    {
      palTogglePad(GPIOC, 13);

      // Process
      ret = usbProtoReadCmd((BaseChannel *)&SDU1);

      // Clear buffer
      chnReadTimeout((BaseChannel *)&SDU1, clear_data, USB_MAX_PACKET_SIZE, TIME_IMMEDIATE);

      // Reply status
      if(ret != MASK_REPLY_INFO)
        chnPutTimeout((BaseChannel *)&SDU1, ret, MS2ST(25));
    }
  }
}


/*
 * Application entry point.
 */
int __attribute__((noreturn)) main(void) {
//int main(void) {
  thread_t *usbThread;

  Get_SerialNum();
  
  while(true)
  {
    halInit();
    chSysInit();
    
    DMXConfig dmx1Config = { 0, &UARTD1, { GPIOA, 9, 10 }, {GPIOA, 11 }, { GPIOB, 7 }, { GPIOA, 14 } };
    DMXConfig dmx2Config = { 1, &UARTD2, { GPIOA, 2, 3 }, { GPIOA, 4 }, { GPIOB, 8}, { GPIOA, 6} };
    DMXConfig dmx3Config = { 2, &UARTD3, { GPIOB, 10, 11 }, { GPIOB, 12 }, { GPIOB, 9 }, { GPIOB, 14} };
    
    // Start USB Thread
    usbThread = chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);

    dmxInit(&dmx1Config);
    dmxStart(&dmx1Config);

    dmxInit(&dmx2Config);
    dmxStart(&dmx2Config);

    dmxInit(&dmx3Config);
    dmxStart(&dmx3Config);
    
    //while (!gDoShutdown)
    while(true)
    {
      chThdSleepMilliseconds(1000);
    }

    // Shutdown and reset

    // Wait a bit before shutting everything down
    //chThdSleepMilliseconds(500);

    // Stop USB Thread
    chThdTerminate(usbThread);
    chThdWait(usbThread);

    dmxStop(&dmx1Config);
    dmxStop(&dmx2Config);
    dmxStop(&dmx3Config);

    /*
    chSysDisable();
    chSysEnable();
    chSysDisable();

    //RCC->CIR; // Disable ALL Interrupts
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    // Disable
    rccDisablePWRInterface(FALSE);
    rccDisableBKPInterface(FALSE);

    // Reset All peripherals
    rccResetAPB1(0xFFFFFFFF);
    rccResetAPB2(0xFFFFFFFF);

    RCC->CFGR = 0;

    //SCB->AIRCR = (0x5FA << SCB_AIRCR_VECTKEY_Pos) |
    //    SCB_AIRCR_SYSRESETREQ;
    NVIC_SystemReset();
    */
  }
}
