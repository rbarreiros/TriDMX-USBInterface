
#include <hal.h>
#include <string.h>
#include "dmx.h"
#include "usbdrv.h"

uint16_t dmxBRR = 0, brkBRR = 0;
DMXPinConfig dmxPinCfg[3];
uint8_t dmxStream[3][DMX_BUFFER_SIZE];

volatile uint8_t dmxStatus[3] = {0, 0, 0};

void dmxProcessTransferComplete(UARTDriver *uart);

/**
 *
 * UARTConfig uartCfg = {
 * txend1_cb - NULL,   // End of Transmission buffer Callback
 * txend2_cb - NULL,   // Physical end of transmission callback
 * rxend_cb - NULL,   // Received buffer filled callback
 * rxchar_cb - NULL,   // Char received while out of the UART_RECEIVE state (to use when receiving dmx)
 * rxerr_cb - NULL,   // Receive error callback
 * speed - DMX_BAUDRATE, // Baudrate
 * cr1 - 0, // cr1 register values
 * cr2 - USART_CR2_STOP_1, // cr2 register values
 * cr3 - 0, // cr3 register values
 * };
 */

UARTConfig uartCfg[3] = {
  { NULL, NULL, NULL, NULL, NULL, DMX_BAUDRATE, 0, USART_CR2_STOP_1, 0 },
  { NULL, NULL, NULL, NULL, NULL, DMX_BAUDRATE, 0, USART_CR2_STOP_1, 0 },
  { NULL, NULL, NULL, NULL, NULL, DMX_BAUDRATE, 0, USART_CR2_STOP_1, 0 }
};

void dmxProcessError(UARTDriver *uart, uartflags_t flag)
{
  uint8_t id = 0;
  
  if(uart == &UARTD1)
    id = 0;
  else if(uart == &UARTD2)
    id = 1;
  else if(uart == &UARTD3)
    id = 2;
  else
    return;
    
  if(flag == UART_FRAMING_ERROR)
  {
    palTogglePad(GPIOC, 13);
    chSysLockFromISR();
    uartStartReceiveI(uart, DMX_BUFFER_SIZE, dmxStream[id]);
    chSysUnlockFromISR();
  }
  
  return;
}

void dmxProcessTransferComplete(UARTDriver *uart)
{
  uint8_t id = 0;

  if(uart == &UARTD1)
    id = 0;
  else if(uart == &UARTD2)
    id = 1;
  else if(uart == &UARTD3)
    id = 2;
  else
    return;
  
  switch(dmxStatus[id])
  {
    case BREAK: // Break finished, send DMX Stream
      dmxStatus[id] = IDLE;

      uart->usart->BRR = dmxBRR;
      chSysLockFromISR();
      uartStartSendI(uart, DMX_BUFFER_SIZE, dmxStream[id]);
      chSysUnlockFromISR();

      break;
    case IDLE:  // Finished sending stream, send break
      dmxStatus[id] = BREAK;
      palTogglePad(dmxPinCfg[id].ledout_pad.port, dmxPinCfg[id].ledout_pad.pad); // DMX 1 Led Togle

      uart->usart->BRR = brkBRR;
      uint8_t tmp[1] = {0};
      chSysLockFromISR();
      uartStartSendI(uart, 1, &tmp);
      chSysUnlockFromISR();
      break;
  };
}

void dmxInit(DMXConfig *cfg)
{
  dmxStatus[cfg->id] = BREAK;

  dmxPinCfg[cfg->id].dir_pad = cfg->dir_pad;
  dmxPinCfg[cfg->id].ledout_pad = cfg->ledout_pad;
  dmxPinCfg[cfg->id].ledin_pad = cfg->ledin_pad;
  
  // Force pal mode here, ignore the board.h/board.c
  palSetPadMode(cfg->uart_pad.port, cfg->uart_pad.pad_tx, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetPadMode(cfg->uart_pad.port, cfg->uart_pad.pad_rx, PAL_MODE_INPUT);

  // Setup Dir and LED Pins
  palSetPadMode(cfg->dir_pad.port, cfg->dir_pad.pad, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(cfg->ledout_pad.port, cfg->ledout_pad.pad, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(cfg->ledin_pad.port, cfg->ledin_pad.pad, PAL_MODE_OUTPUT_PUSHPULL);

  // Start and stop Uart to setup the BRR's
  uartStart(cfg->driver, &uartCfg[cfg->id]);
  if(dmxBRR == 0) dmxBRR = cfg->driver->usart->BRR;
  uartStop(cfg->driver);

  uartCfg[cfg->id].speed = BREAK_BAUDRATE;

  uartStart(cfg->driver, &uartCfg[cfg->id]);
  if(brkBRR == 0) brkBRR = cfg->driver->usart->BRR;
  uartStop(cfg->driver);

  // Init DMX Buffer
  unsigned int i;
  for(i = 0; i < DMX_BUFFER_SIZE; i++)
  {
    dmxStream[cfg->id][i] = 0;
  }
  
}

void dmxStart(DMXConfig *cfg)
{
  switch(cfg->cfg.direction)
  {
    case DMX_OUT: // Output
      uartCfg[cfg->id].txend2_cb = &dmxProcessTransferComplete;
      
      // Make sure the driver is stopped
      uartStop(cfg->driver);
      uartStart(cfg->driver, &uartCfg[cfg->id]);

      // Starts the automated dmx send process
      uint8_t tmp[1] = {0};
      uartStartSend(cfg->driver, 1, &tmp);
      break;
    case DMX_IN: // Input
      dmxStop(cfg);
      
      uartCfg[cfg->id].rxerr_cb = &dmxProcessError;
      uartCfg[cfg->id].speed = DMX_BAUDRATE;
      
      uartStart(cfg->driver, &uartCfg[cfg->id]);
      break;
    case DMX_MERGE: // Merge Output
      // setup this driver as output
      // configure source a to sum source b
      // place result in dmxStream
      break;
    case DMX_MIRROR: // Mirror
      // setup this driver as output
      // copy source dmxStream to this dmxStream
      break;
  }
}

void dmxStop(DMXConfig *cfg)
{
  uartStop(cfg->driver);
  uartCfg[cfg->id].txend2_cb = NULL;
  uartCfg[cfg->id].rxerr_cb = NULL;
  uartCfg[cfg->id].rxend_cb = NULL;
}

void dmxSetDirection(DMXConfig *cfg, eDmxDirection dir)
{
  if(dir > DMX_MERGE) return;
  
  cfg->cfg.direction = dir;
  dmxStop(cfg);
  dmxStart(cfg);
}

void dmxSetChannel(uint8_t port, uint16_t channel, uint8_t value)
{
  if(port > 2) return;
  if((channel + 1) > DMX_BUFFER_SIZE) return;
  dmxStream[port][channel + 1] = value & 0xff;
}

uint8_t dmxUpdate(uint8_t port, uint8_t *data, uint8_t len)
{
  if(port > 2) return 1;

  // Data needs to be divisible by 2, as it's chn/value pairs
  if( len % 2 != 0 ) return 1;

  int i;
  for(i = 0; i < len; i += 2)
  {
    // Let's not forget DMX starts at index 1 (0 is SC)
    // While channels received by the device are 0 based
    if( (data[i] + 1) < DMX_BUFFER_SIZE)
      dmxStream[port][data[i] + 1] = data[i + 1];
  }

  return 0;
}

uint8_t dmxSetStream(uint8_t port, uint8_t *data, uint8_t len, uint8_t start)
{
  if(port > 2) return 1;

  static unsigned int lastAddr[3] = {1, 1, 1};

  if(start)
    lastAddr[port] = 1;

  // This should NEVER happen if all USB comms are OK
  if(lastAddr[port] + len > DMX_BUFFER_SIZE)
    return 1;
  
  memcpy(&dmxStream[port][lastAddr[port]], data, len);
  lastAddr[port] += len;

  return 0;
}

void dmxGetStream(uint8_t port, uint8_t *data, uint8_t len, uint8_t start)
{
  (void)port;
  (void)data;
  (void)len;
  (void)start;
}

uint8_t dmxGetChannel(uint8_t port, uint16_t channel)
{
  if(port < 1 || port > 3) return 0;
  if(channel > 512) return 0;
  
  return dmxStream[port][channel];
}
