
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ch.h"
#include "hal.h"
#include "usbdrv.h"
#include "usb_protocol.h"
#include "dmx.h"

/*
 * Application entry point.
 */
int __attribute__((noreturn)) main(void) {
//int main(void) {
  thread_t *usbThread;
  thread_t *dmxThread;

  Get_SerialNum();

  while(true)
  {
    halInit();
    chSysInit();

    // Start USB Thread
    usbThread = chThdCreateStatic(waUSB, sizeof(waUSB), NORMALPRIO, USBThread, NULL);

    // Start DMX Thread
    dmxThread = chThdCreateStatic(waDMX, sizeof(waDMX), NORMALPRIO, DMXThread, NULL);
    
    while (!gDoShutdown)
    {
      chThdSleepMilliseconds(1000);
    }

    // Shutdown and reset

    // Wait a bit before shutting everything down
    //chThdSleepMilliseconds(500);

    // Stop USB Thread
    chThdTerminate(usbThread);
    chThdWait(usbThread);

    // Stop DMX Thread
    chThdTerminate(dmxThread);
    chThdWait(dmxThread);
    
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
