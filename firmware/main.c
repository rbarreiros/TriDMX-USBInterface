
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ch.h"
#include "hal.h"
#include "usbdrv.h"
#include "usb_protocol.h"
#include "dmx.h"
#include "config.h"

/*
 * Application entry point.
 */
int __attribute__((noreturn)) main(void) {
//int main(void) {
  thread_t *usbThread;
  thread_t *dmxThread;

  Get_SerialNum();

  // Load configuration
  configLoad();
  
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
    //NVIC_SystemReset();

    // Copy Blackmagic reboot to bootloader method !
    /* Disconnect USB cable by resetting USB Device and pulling USB_DP low*/
    /*
    rcc_periph_reset_pulse(RST_USB);
    rcc_periph_clock_enable(RCC_USB);
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_clear(GPIOA, GPIO12);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
		  GPIO_CNF_OUTPUT_OPENDRAIN, GPIO12);
    */
    rccResetUSB();
    rccEnableUSB();
    // clock enable GPIOA
    palClearPad(12, GPIOA);
    palSetPadMode(12, GPIOA, PAL_MODE_OUTPUT_OPENDRAIN);

    /* Assert bootloader pin */
    /*
    uint32_t crl = GPIOA_CRL;
    rcc_periph_clock_enable(RCC_GPIOA);
    */
    uint32_t crl = GPIOA_CRL;
    // clock enable RCC_GPIOA (already did?)
    
    /* Enable Pull on GPIOA1. We don't rely on the external pin
     * really pulled, but only on the value of the CNF register
     * changed from the reset value
     */
    /*
    crl &= 0xffffff0f;
    crl |= 0x80;
    GPIOA_CRL = crl;
    */

    //Core reset
    // scb_reset_core() from libopencm3
    SCB_AIRCR = SCB_AIRCR_VECTKEY | SCB_AIRCR_VECTRESET;
    while(1);
  }
}
