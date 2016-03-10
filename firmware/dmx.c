
#include <hal.h>
#include "dmx.h"
#include "usbdrv.h"

void dmx1ProcessTransferComplete(UARTDriver *uart);
void dmx2ProcessTransferComplete(UARTDriver *uart);
void dmx3ProcessTransferComplete(UARTDriver *uart);

// Globals

UARTConfig uart1Cfg = {
  NULL,   // End of Transmission buffer Callback
  NULL,   // Physical end of transmission callback
  NULL,   // Received buffer filled callback
  NULL,   // Char received while out of the UART_RECEIVE state (to use when receiving dmx)
  NULL,   // Receive error callback
  DMX_BAUDRATE, // Baudrate
  0, // cr1 register values
  USART_CR2_STOP_1, // cr2 register values
  0, // cr3 register values
};

//UARTConfig uart2Cfg = { NULL, NULL, NULL, NULL, NULL, DMX_BAUDRATE, 0, USART_CR2_STOP_1, 0 };
//UARTConfig uart3Cfg = { NULL, NULL, NULL, NULL, NULL, DMX_BAUDRATE, 0, USART_CR2_STOP_1, 0 };

uint8_t dmx1Stream[DMX_BUFFER_SIZE] = {0};
//uint8_t dmx2Stream[DMX_BUFFER_SIZE] = {0};
//uint8_t dmx3Stream[DMX_BUFFER_SIZE] = {0};

uint8_t dmx1Status = 0;
//uint8_t dmx2Status = 0;
//uint8_t dmx3Status = 0;

uint16_t dmxBRR = 0, brkBRR = 0;

// UART 1 - PA9, PA10

void dmx1Init(void)
{
  dmx1Status = BREAK;

  // Force pal mode here, ignore the board.h/board.c
  palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetPadMode(GPIOA, 10, PAL_MODE_INPUT);

  uartStart(&UARTD1, &uart1Cfg);
  if(dmxBRR == 0) dmxBRR = UARTD1.usart->BRR;

  uart1Cfg.speed = BREAK_BAUDRATE;
  uartStart(&UARTD1, &uart1Cfg);
  if(brkBRR == 0) brkBRR = UARTD1.usart->BRR;

  uartStop(&UARTD1);
}

void dmx1Start(void)
{
  uart1Cfg.txend2_cb = &dmx1ProcessTransferComplete;
  uartStart(&UARTD1, &uart1Cfg);
  uartStartSend(&UARTD1, 1, 0x00);
}

void dmx1Stop(void)
{
  uart1Cfg.txend2_cb = NULL;
  uartStop(&UARTD1);
}

void dmx1SetDirection(eDmxDirection dir)
{
  (void)dir;
}

void dmx1SetChannel(uint16_t channel, uint8_t value)
{
  if(channel > 512 || channel == 0) return;
  dmx1Stream[channel] = value & 0xff;
}

void dmx1ProcessTransferComplete(UARTDriver *uart)
{
  switch(dmx1Status)
  {
    case BREAK: // Break finished, send DMX Stream
      dmx1Status = IDLE;

      uart->usart->BRR = dmxBRR;
      chSysLockFromISR();
      uartStartSendI(uart, DMX_BUFFER_SIZE, dmx1Stream);
      chSysUnlockFromISR();
      
      //palTogglePad(DMX1_GPIO_BANK, DMX1_LED); // DMX 1 Led Togle
      break;
    case IDLE:  // Finished sending stream, send break
      dmx1Status = BREAK;

      uart->usart->BRR = brkBRR;
      chSysLockFromISR();
      uartStartSendI(uart, 1, 0x00);
      chSysUnlockFromISR();
      break;
  };
}

// UART 2 - PA2, PA3
/*
void dmx2Init(void)
{
  dmx2Status = BREAK;

  // Force pal mode here, ignore the board.h/board.c
  palSetPadMode(GPIOA, 2, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetPadMode(GPIOA, 3, PAL_MODE_INPUT);
  
  uartStart(&UARTD2, &uart2Cfg);
  if(dmxBRR == 0) dmxBRR = UARTD2.usart->BRR;

  uart2Cfg.speed = BREAK_BAUDRATE;
  uartStart(&UARTD2, &uart2Cfg);
  if(brkBRR == 0) brkBRR = UARTD2.usart->BRR;

  uartStop(&UARTD2);
}

void dmx2Start(void)
{
  uart2Cfg.txend2_cb = &dmx2ProcessTransferComplete;
  uartStart(&UARTD2, &uart2Cfg);
  uartStartSend(&UARTD2, 1, 0x00);
}

void dmx2Stop(void)
{
  uart2Cfg.txend2_cb = NULL;
  uartStop(&UARTD2);
}

void dmx2SetDirection(eDmxDirection dir)
{
  (void)dir;
}

void dmx2SetChannel(uint16_t channel, uint8_t value)
{
  if(channel > 512 || channel == 0) return;
  dmx2Stream[channel] = value & 0xff;
}

void dmx2ProcessTransferComplete(UARTDriver *uart)
{
  switch(dmx2Status)
  {
    case BREAK: // Break finished, send DMX Stream
      dmx2Status = IDLE;

      uart->usart->BRR = dmxBRR;
      chSysLockFromISR();
      uartStartSendI(uart, DMX_BUFFER_SIZE, dmx2Stream);
      chSysUnlockFromISR();
      
      //palTogglePad(DMX2_GPIO_BANK, DMX2_LED); // DMX 2 Led Togle
      break;
    case IDLE:  // Finished sending stream, send break
      dmx2Status = BREAK;

      uart->usart->BRR = brkBRR;
      chSysLockFromISR();
      uartStartSendI(uart, 1, 0x00);
      chSysUnlockFromISR();
      break;
  };
}

// UART 3 - PB10, PB11

void dmx3Init(void)
{
  dmx3Status = BREAK;

  // Force pal mode here, ignore the board.h/board.c
  palSetPadMode(GPIOB, 10, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetPadMode(GPIOB, 11, PAL_MODE_INPUT);
  
  uartStart(&UARTD3, &uart3Cfg);
  if(dmxBRR == 0) dmxBRR = UARTD3.usart->BRR;

  uart3Cfg.speed = BREAK_BAUDRATE;
  uartStart(&UARTD3, &uart3Cfg);
  if(brkBRR == 0) brkBRR = UARTD3.usart->BRR;

  uartStop(&UARTD3);
}

void dmx3Start(void)
{
  uart3Cfg.txend2_cb = &dmx3ProcessTransferComplete;
  uartStart(&UARTD3, &uart3Cfg);
  uartStartSend(&UARTD3, 1, 0x00);
}

void dmx3Stop(void)
{
  uart3Cfg.txend2_cb = NULL;
  uartStop(&UARTD3);
}

void dmx3SetDirection(eDmxDirection dir)
{
  (void)dir;
}

void dmx3SetChannel(uint16_t channel, uint8_t value)
{
  if(channel > 512 || channel == 0) return;
  dmx3Stream[channel] = value & 0xff;
}

void dmx3ProcessTransferComplete(UARTDriver *uart)
{
  switch(dmx3Status)
  {
    case BREAK: // Break finished, send DMX Stream
      dmx3Status = IDLE;

      uart->usart->BRR = dmxBRR;
      chSysLockFromISR();
      uartStartSendI(uart, DMX_BUFFER_SIZE, dmx3Stream);
      chSysUnlockFromISR();

      //palTogglePad(DMX3_GPIO_BANK, DMX3_LED); // DMX 3 Led Togle
      break;
    case IDLE:  // Finished sending stream, send break
      dmx3Status = BREAK;

      uart->usart->BRR = brkBRR;
      chSysLockFromISR();
      uartStartSendI(uart, 1, 0x00);
      chSysUnlockFromISR();
      break;
  };
}
*/
// Functions used by USB to update the DMX Stream or read from

uint8_t dmxUpdate(uint8_t *data, uint8_t len)
{
  // First array item is the Port ( 1 based, so 1, 2 or 3 )
  if(data[0] < 1 || data[0] > 3)
    return 1;

  // Data needs to be divisible by 2, as it's chn/value pairs
  if( (len - 1) % 2 != 0 )
    return 1;

  int i;
  for(i = 1; i < (len -1); i += 2)
  {
    if(data[0] == 1)
      dmx1SetChannel(data[i], data[i + 1]);
    //else if(data[0] == 2)
      //dmx2SetChannel(data[i], data[i + 1]);
    //else if(data[0] == 3)
      //dmx3SetChannel(data[i], data[i + 1]);
  }

  return 0;
}
