
#include <hal.h>
#include <string.h>
#include "config.h"
#include "dmx.h"
#include "usbdrv.h"

uint16_t dmxBRR = 0, brkBRR = 0;
uint8_t dmxStream[3][DMX_BUFFER_SIZE];

volatile uint8_t dmxStatus[3] = {0, 0, 0};

// Default Values
// This needs to be populated with the saved eeprom values
// from the last used configuration, used to setup DMX Ports
// when starting
DMXConfig dmxConfig[3] = {
  { 0, &UARTD1,
    { DMX1_UART_PORT, DMX1_UART_PAD_TX, DMX1_UART_PAD_RX },
    { DMX1_DIRECTION_PORT, DMX1_DIRECTION_PAD },
    { DMX1_LEDOUT_PORT, DMX1_LEDOUT_PAD },
    { DMX1_LEDIN_PORT, DMX1_LEDIN_PAD },
    { DIRECTION_OUTPUT, SOURCE_USB, SOURCE_PORT2, MERGE_HTP, SOURCE_USB }, false, false, 0 },
  { 1, &UARTD2,
    { DMX2_UART_PORT, DMX2_UART_PAD_TX, DMX2_UART_PAD_RX },
    { DMX2_DIRECTION_PORT, DMX2_DIRECTION_PAD },
    { DMX2_LEDOUT_PORT, DMX2_LEDOUT_PAD },
    { DMX2_LEDIN_PORT, DMX2_LEDIN_PAD },
    { DIRECTION_OUTPUT, SOURCE_USB, SOURCE_PORT3, MERGE_HTP, SOURCE_USB }, false, false, 0 },
  { 2, &UARTD3,
    { DMX3_UART_PORT, DMX3_UART_PAD_TX, DMX3_UART_PAD_RX },
    { DMX3_DIRECTION_PORT, DMX3_DIRECTION_PAD },
    { DMX3_LEDOUT_PORT, DMX3_LEDOUT_PAD },
    { DMX3_LEDIN_PORT, DMX3_LEDIN_PAD },
    { DIRECTION_OUTPUT, SOURCE_USB, SOURCE_PORT1, MERGE_HTP, SOURCE_USB }, false, false, 0 }
};

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

// Used to signal the reception completion of the dmx stream
void dmxProcessReceiveEnd(UARTDriver *uart)
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
  (void)id;
  return;
}

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
    // Toggle input led
    if(!dmxConfig[id].id_enabled)
      palTogglePad(dmxConfig[id].ledin_pad.port, dmxConfig[id].ledin_pad.pad);

    // Start DMA DMX Read
    chSysLockFromISR();
    uartStartReceiveI(uart, DMX_BUFFER_SIZE, dmxStream[id]);
    chSysUnlockFromISR();
    dmxConfig[id].stream_ts = chVTGetSystemTimeX();
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
      dmxConfig[id].stream_ts = chVTGetSystemTimeX();

      break;
    case IDLE:  // Finished sending stream, send break
      dmxStatus[id] = BREAK;

      // Toggle output led
      if(!dmxConfig[id].id_enabled)
        palTogglePad(dmxConfig[id].ledout_pad.port, dmxConfig[id].ledout_pad.pad);

      uart->usart->BRR = brkBRR;
      uint8_t tmp[1] = {0};
      chSysLockFromISR();
      uartStartSendI(uart, 1, &tmp);
      chSysUnlockFromISR();
      break;
  };
}

void dmxInit(uint8_t id)
{
  dmxStatus[id] = IDLE;

  // Force pal mode here, ignore the board.h/board.c
  palSetPadMode(dmxConfig[id].uart_pad.port, dmxConfig[id].uart_pad.pad_tx, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetPadMode(dmxConfig[id].uart_pad.port, dmxConfig[id].uart_pad.pad_rx, PAL_MODE_INPUT);

  // Setup Dir and LED Pins
  palSetPadMode(dmxConfig[id].dir_pad.port, dmxConfig[id].dir_pad.pad, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(dmxConfig[id].ledout_pad.port, dmxConfig[id].ledout_pad.pad, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(dmxConfig[id].ledin_pad.port, dmxConfig[id].ledin_pad.pad, PAL_MODE_OUTPUT_PUSHPULL);

  palSetPad(dmxConfig[id].dir_pad.port, dmxConfig[id].dir_pad.pad);
  palClearPad(dmxConfig[id].ledin_pad.port, dmxConfig[id].ledin_pad.pad);
  palClearPad(dmxConfig[id].ledout_pad.port, dmxConfig[id].ledout_pad.pad);
  
  // Start and stop Uart to setup the BRR's
  uartStart(dmxConfig[id].driver, &uartCfg[id]);
  if(dmxBRR == 0) dmxBRR = dmxConfig[id].driver->usart->BRR;
  uartStop(dmxConfig[id].driver);

  uartCfg[id].speed = BREAK_BAUDRATE;

  uartStart(dmxConfig[id].driver, &uartCfg[id]);
  if(brkBRR == 0) brkBRR = dmxConfig[id].driver->usart->BRR;
  uartStop(dmxConfig[id].driver);

  // Init DMX Buffer
  unsigned int i;
  for(i = 0; i < DMX_BUFFER_SIZE; i++)
  {
    dmxStream[id][i] = 0;
  }
  
}

void dmxStart(uint8_t id)
{
  if(dmxConfig[id].started)
    return;
  
  switch(dmxConfig[id].cfg.direction)
  {
    // hmmm when merging self + other
    // shouldn't we have a temp intermediary
    // buffer to hold self from USB then merge
    // before sending straight away ? hmmm
    case DIRECTION_MERGE: // Merge Output
    case DIRECTION_MIRROR: // Mirror
    case DIRECTION_OUTPUT: // Output
      dmxStop(id);

      palSetPad(dmxConfig[id].dir_pad.port,
                dmxConfig[id].dir_pad.pad);

      uartCfg[id].txend2_cb = &dmxProcessTransferComplete;
      
      // Make sure the driver is stopped
      uartStop(dmxConfig[id].driver);
      uartStart(dmxConfig[id].driver, &uartCfg[id]);

      // Starts the automated dmx send process
      uint8_t tmp[1] = {0};
      uartStartSend(dmxConfig[id].driver, 1, &tmp);
      break;
    case DIRECTION_INPUT: // Input
      dmxStop(id);

      palClearPad(dmxConfig[id].dir_pad.port,
                  dmxConfig[id].dir_pad.pad);

      uartCfg[id].rxend_cb = &dmxProcessReceiveEnd;
      uartCfg[id].rxerr_cb = &dmxProcessError;
      uartCfg[id].speed = DMX_BAUDRATE;
      
      uartStart(dmxConfig[id].driver, &uartCfg[id]);
      break;
    case DIRECTION_MAX:
    default:
      break;
  }

  dmxConfig[id].started = true;
}

void dmxStop(uint8_t id)
{
  uartStop(dmxConfig[id].driver);
  uartCfg[id].txend2_cb = NULL;
  uartCfg[id].rxerr_cb = NULL;
  uartCfg[id].rxend_cb = NULL;
  dmxConfig[id].started = false;
}

bool dmxSetDirection(uint8_t id, eDmxDirection dir)
{
  if(dir >= DIRECTION_MAX) return false;
  
  dmxConfig[id].cfg.direction = dir;
  if(dmxConfig[id].started)
  {
    dmxStop(id);
    dmxStart(id);
  }

  return true;
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

uint8_t dmxGetStream(uint8_t port, uint8_t *data, uint8_t len)
{
  memset(data, 0, len);
  memcpy(data, dmxStream[port], len);
  
  return 0;
}

uint8_t dmxGetChannel(uint8_t port, uint16_t channel)
{
  if(port < 1 || port > 3) return 0;
  if(channel > 512) return 0;
  
  return dmxStream[port][channel];
}

DMXPortConfig dmxGetPortConfig(uint8_t port)
{
  if(port > 2)
  {
    DMXPortConfig d = { -1, -1, -1, -1, -1 };
    return d;
  }
  
  return dmxConfig[port].cfg;
}

bool dmxSetPortConfig(uint8_t port, DMXPortConfig *cfg)
{
  if(port > 2)
    return false;
  
  bool changeDir = false;
  bool ret = true;
  
  if(dmxConfig[port].cfg.direction != cfg->direction)
    changeDir = true;

  memcpy(&dmxConfig[port].cfg, cfg, sizeof(DMXPortConfig));

  if(changeDir)
    ret = dmxSetDirection(port, cfg->direction);

  return ret;
}

// Identify Thread
static THD_FUNCTION(DMXIDThread, arg)
{
  uint8_t id = *((uint8_t*)arg);

  dmxConfig[id].id_enabled = true;
  
  palClearPad(dmxConfig[id].ledout_pad.port, dmxConfig[id].ledout_pad.pad);
  palClearPad(dmxConfig[id].ledin_pad.port, dmxConfig[id].ledin_pad.pad);

  while(!chThdShouldTerminateX())
  {
    palTogglePad(dmxConfig[id].ledout_pad.port, dmxConfig[id].ledout_pad.pad);
    palTogglePad(dmxConfig[id].ledin_pad.port, dmxConfig[id].ledin_pad.pad);
    chThdSleepMilliseconds(500);        
  }

  palClearPad(dmxConfig[id].ledout_pad.port, dmxConfig[id].ledout_pad.pad);
  palClearPad(dmxConfig[id].ledin_pad.port, dmxConfig[id].ledin_pad.pad);

  dmxConfig[id].id_enabled = false;
}

void dmxIdentify(uint8_t port)
{
  static thread_t *blinkThreads[3];

  if(port > 2) return;
  
  if(!dmxConfig[port].id_enabled)
  {
    blinkThreads[port] = chThdCreateFromHeap(NULL, THD_WORKING_AREA_SIZE(64),
					     "ID Thread", NORMALPRIO, DMXIDThread, &port);
  }
  else
  {
    chThdTerminate(blinkThreads[port]);
    chThdWait(blinkThreads[port]);
  }
}

// DMX Thread
THD_WORKING_AREA(waDMX, DMX_THREAD_SIZE);
THD_FUNCTION(DMXThread, arg)
{
  uint8_t i;
  (void)arg;
  
  for(i = 0; i < 3; i++)
  {
    dmxInit(i);
    dmxStart(i);
  }

  while(!chThdShouldTerminateX())
  {
    for(i = 0; i < 3; i++)
    {
      // Process Mirror
      if(dmxConfig[i].cfg.direction == DIRECTION_MIRROR)
      {
	uint8_t src = dmxConfig[i].cfg.merge_source_a;
	if(src > 2 || src == i)
	  continue;
	else
	  memcpy(dmxStream[i], dmxStream[src], DMX_BUFFER_SIZE);
      }
      else if(dmxConfig[i].cfg.direction == DIRECTION_MERGE)
      {
	uint8_t src_a = dmxConfig[i].cfg.merge_source_a;
	uint8_t src_b = dmxConfig[i].cfg.merge_source_b;
	
	if(src_a == src_b)
	  continue;
	else if(src_a > 2 || src_b > 2)
	  continue;

	// if src_a == i or src_b == i then we're merging our
	// port with other, our being either input or usb
	
	// Merge using HTP
	if(dmxConfig[i].cfg.merge_htp_ltp == MERGE_HTP)
	{
	  unsigned int j;
	  for(j = 0; j < DMX_BUFFER_SIZE; j++)
	  {
	    uint8_t val = 0;
	    if(dmxStream[src_a][j] > dmxStream[src_b][j])
	      val = dmxStream[src_a][j];
	    else if(dmxStream[src_b][j] > dmxStream[src_a][j])
	      val = dmxStream[src_b][j];
	    else
	      val = dmxStream[src_a][j];

	    dmxStream[i][j] = val;
	  }
	}
	else if(dmxConfig[i].cfg.merge_htp_ltp == MERGE_LTP)
	{
	  if(dmxConfig[src_a].stream_ts > dmxConfig[src_b].stream_ts)
	    memcpy(dmxStream[i], dmxStream[src_a], DMX_BUFFER_SIZE);
	  else
	    memcpy(dmxStream[i], dmxStream[src_b], DMX_BUFFER_SIZE);
	}
      }
    }
    
    chThdSleepMilliseconds(1);
  }

  for(i = 0; i < 3; i++)
    dmxStop(i);
}
