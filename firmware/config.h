#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <hal.h>
#include "dmx.h"

// *** DMX Configuration *** //

// WARNING WARNING WARNING WARNING WARNING WARNING WARNING //
//
// DO NOT USE GPIOA 11 AND 12 THEY ARE USED IN USB         //
// DO NOT USE GPIOA 1 BOOTLOADER TRIGGER PIN               //
// DO NOT USE GPIOB 2 - BOOT1                              //
// DO NOT USE GPIOB 3 and 4 - JTDO and JNTRST recpectively //
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING //

// DMX 1

#define DMX1_UART_PORT      GPIOA
#define DMX1_UART_PAD_TX    9
#define DMX1_UART_PAD_RX    10

#define DMX1_DIRECTION_PORT GPIOA
#define DMX1_DIRECTION_PAD  8

#define DMX1_LEDOUT_PORT    GPIOB
#define DMX1_LEDOUT_PAD     13

#define DMX1_LEDIN_PORT     GPIOB
#define DMX1_LEDIN_PAD      14

// DMX 2

#define DMX2_UART_PORT      GPIOA
#define DMX2_UART_PAD_TX    2
#define DMX2_UART_PAD_RX    3

#define DMX2_DIRECTION_PORT GPIOA
#define DMX2_DIRECTION_PAD  4

#define DMX2_LEDOUT_PORT    GPIOB
#define DMX2_LEDOUT_PAD     6

#define DMX2_LEDIN_PORT     GPIOB
#define DMX2_LEDIN_PAD      7

// DMX 3

#define DMX3_UART_PORT      GPIOB
#define DMX3_UART_PAD_TX    10
#define DMX3_UART_PAD_RX    11

#define DMX3_DIRECTION_PORT GPIOB
#define DMX3_DIRECTION_PAD  1

#define DMX3_LEDOUT_PORT    GPIOB
#define DMX3_LEDOUT_PAD     8

#define DMX3_LEDIN_PORT     GPIOB
#define DMX3_LEDIN_PAD      9

// Board LEDS

#define USB_STATUS_LED_PORT GPIOC
#define USB_STATUS_LED_PAD  13

// *** NO USER SERVICEABLE PARTS BELOW! *** //

// Whole device configuration
// used to save config in eeprom
typedef struct
{
  DMXPortConfig port[3];
  // ... Other future options
} DeviceConfig;


void configLoad(void);
DMXPortConfig configGetPortConfig(uint8_t port);
bool configSetPortConfig(uint8_t port, DMXPortConfig *cfg);

#endif
