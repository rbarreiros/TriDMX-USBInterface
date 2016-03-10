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
  
  (void)arg;
  chRegSetThreadName("USB");

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
int __attribute__((noreturn)) main(void) {
  while(TRUE)
  {
    halInit();
    chSysInit();

    // Start DMX Streams
    dmx1Init();
    dmx1Start();

    /*
    dmx2Init();
    dmx2Start();
    
    dmx3Init();
    dmx3Start();
    */
    
    // Start USB Thread
    thread_t *usbThread = chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);
    
    while (!gDoShutdown) {
      chThdSleepMilliseconds(1000);
    }

    // Shutdown and reset
    
    // Wait a bit before shutting everything down
    chThdSleepMilliseconds(500);
    
    // Stop all DMX
    dmx1Stop();
    //dmx2Stop();
    //dmx3Stop();
    
    // Stop USB Thread
    chThdTerminate(usbThread);
    chThdWait(usbThread);
    
    chSysDisable();
    chSysEnable();
    chSysDisable();
    
    RCC->CIR; // Disable ALL Interrupts
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
