
#include <hal.h>
#include "dmx.h"
#include "usbdrv.h"

uint16_t dmxBRR = 0, brkBRR = 0;
uint8_t dmxStream[3][DMX_BUFFER_SIZE];
uint8_t dmxStatus[3] = {0, 0, 0};
DMXPinConfig dmxPinCfg[3];

void dmxProcessTransferComplete(UARTDriver *uart);

/**
 *
 * UARTConfig uartCfg = {
 * NULL,   // End of Transmission buffer Callback
 * NULL,   // Physical end of transmission callback
 * NULL,   // Received buffer filled callback
 * NULL,   // Char received while out of the UART_RECEIVE state (to use when receiving dmx)
 * NULL,   // Receive error callback
 * DMX_BAUDRATE, // Baudrate
 * 0, // cr1 register values
 * USART_CR2_STOP_1, // cr2 register values
 * 0, // cr3 register values
 * };
 */

UARTConfig uartCfg[3] = {
  { NULL, &dmxProcessTransferComplete, NULL, NULL, NULL, DMX_BAUDRATE, 0, USART_CR2_STOP_1, 0 },
  { NULL, &dmxProcessTransferComplete, NULL, NULL, NULL, DMX_BAUDRATE, 0, USART_CR2_STOP_1, 0 },
  { NULL, &dmxProcessTransferComplete, NULL, NULL, NULL, DMX_BAUDRATE, 0, USART_CR2_STOP_1, 0 }
};

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
  
  uartStart(cfg->driver, &uartCfg[cfg->id]);
  if(dmxBRR == 0) dmxBRR = cfg->driver->usart->BRR;

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
  uint8_t tmp[1] = {0};

  // Make sure the driver is stopped
  uartStop(cfg->driver);
  uartStart(cfg->driver, &uartCfg[cfg->id]);
  uartStartSend(cfg->driver, 1, &tmp);
}

void dmxStop(DMXConfig *cfg)
{
  uartStop(cfg->driver);
}

void dmxSetDirection(DMXConfig *cfg, eDmxDirection dir)
{
  (void)cfg;
  (void)dir;
}

void dmxSetChannel(uint8_t port, uint16_t channel, uint8_t value)
{
  if(port > 3) return;
  if(channel > 512 || channel == 0) return;
  dmxStream[port][channel] = value & 0xff;
}

// Functions used by USB to update the DMX Stream or read from
uint8_t dmxUpdate(uint8_t port, uint8_t *data, uint8_t len)
{
  if(port > 2) return 1;

  // Data needs to be divisible by 2, as it's chn/value pairs
  if( len % 2 != 0 ) return 1;

  int i;
  for(i = 0; i < len; i += 2)
  {
    dmxSetChannel(port, data[i], data[i + 1]);
  }

  return 0;
}

uint8_t dmxSetStream(uint8_t port, uint8_t *data, uint8_t len, uint8_t start)
{
  if(port > 2) return 1;

  int i;
  static unsigned int lastAddr[3] = {1, 1, 1};

  if(start)
    lastAddr[port] = 1;
  
  for(i = 0; i < len; i++)
  {
    chSysLock();
    dmxStream[port][lastAddr[port]] = data[i];
    chSysUnlock();
    
    lastAddr[port]++;
    if(lastAddr[port] > (DMX_BUFFER_SIZE - 1))
    {
      lastAddr[port] = 1;
      return 1;
    }
  }

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
