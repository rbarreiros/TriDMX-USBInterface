
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
    chThdSleepMilliseconds(100);

    // Stop DMX Thread
    chThdTerminate(dmxThread);
    chThdWait(dmxThread);

    // Stop USB Thread
    chThdTerminate(usbThread);
    chThdWait(usbThread);

    chSysDisable();
    chSysEnable();
    chSysDisable();

    // Reboot
    NVIC_SystemReset();
  }
}
