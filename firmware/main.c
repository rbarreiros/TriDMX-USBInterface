/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

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

  chRegSetThreadName("USB");
  
  (void)arg;

  /*
   * Initializes a Bulk USB driver.
   */
  sduObjectInit(&SDU1);
  sduStart(&SDU1, &serusbcfg);

  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1500);
  usbStart(&USBD1, &usbcfg);
  usbConnectBus(serusbcfg.usbp);

  chEvtRegisterMask(chnGetEventSource(&SDU1), &el1, ALL_EVENTS);

  while(USBD1.state != USB_READY) chThdSleepMilliseconds(10);
  while(SDU1.state != SDU_READY) chThdSleepMilliseconds(10);

  while(!chThdShouldTerminateX())
  {
    chEvtWaitAny(ALL_EVENTS);
    chSysLock();
    flags = chEvtGetAndClearFlagsI(&el1);
    chSysUnlock();

    if (flags & CHN_INPUT_AVAILABLE)
    {
      uint8_t cmd_res = usbProtoReadCmd((BaseChannel *)&SDU1);
      (void)cmd_res; // Not used for now
      //chnReadTimeout((BaseChannel *)&SDU1, clear_buff, 64, MS2ST(25) );
    }
  }

  // Shutdown was issued
  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1500);
  //usbStop(&USBD1);
  //sduStop(&SDU1);

  // Force USB Disconnect driving PA12 low for a bit
  // *** WARNING *** NEEDS Pull-up on PA12 CAN DAMAGE STUFF
  /*
  palSetPadMode(GPIOA, 12, PAL_MODE_OUTPUT_OPENDRAIN);
  palClearPad(GPIOA, 12);
  chThdSleepMilliseconds(500);
  palSetPadMode(GPIOA, 12, PAL_MODE_INPUT);
  chThdSleepMilliseconds(500);
  */
  
  return;
}


/*
 * Application entry point.
 */
//int __attribute__((noreturn)) main(void) {
int main(void) {
  thread_t *usbThread;
  
  while(TRUE)
  {
    halInit();
    chSysInit();
    
    DMXConfig dmx1Config = { 0, &UARTD1, { GPIOA, 9, 10 }, {GPIOA, 11 }, { GPIOC, 13 }, { GPIOA, 14 } };
    DMXConfig dmx2Config = { 1, &UARTD2, { GPIOA, 2, 3 }, { GPIOA, 4 }, { GPIOA, 5}, { GPIOA, 6} };
    DMXConfig dmx3Config = { 2, &UARTD3, { GPIOB, 10, 11 }, { GPIOB, 12 }, { GPIOB, 13 }, { GPIOB, 14} };
    
    // Start USB Thread
    usbThread = chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);

    dmxInit(&dmx1Config);
    dmxStart(&dmx1Config);

    dmxInit(&dmx2Config);
    dmxStart(&dmx2Config);

    dmxInit(&dmx3Config);
    dmxStart(&dmx3Config);
    
    while (!gDoShutdown) {
      chThdSleepMilliseconds(10);
    }

    // Shutdown and reset

    // Wait a bit before shutting everything down
    chThdSleepMilliseconds(500);

    // Stop USB Thread
    chThdTerminate(usbThread);
    chThdWait(usbThread);

    dmxStop(&dmx1Config);
    dmxStop(&dmx2Config);
    dmxStop(&dmx3Config);
    
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
  }
}
