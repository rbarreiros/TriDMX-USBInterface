#ifndef __DMX_H__
#define __DMX_H__

#define DMX_BUFFER_SIZE 513
#define DMX_BAUDRATE    250000 // 375000
#define BREAK_BAUDRATE  75000  // 110000

#define DMX1_GPIO_BANK GPIOC
#define DMX1_LED       13

#define DMX1_DIR_GPIO  GPIOC
#define DMX1_DIR_PIN   1

typedef struct {
  ioportid_t port;
  uint8_t pad_tx;
  uint8_t pad_rx;
} DMXUartPin;

typedef struct {
  ioportid_t port;
  uint8_t pad;
} DMXPin;

typedef struct {
  DMXPin dir_pad;
  DMXPin ledout_pad;
  DMXPin ledin_pad;
} DMXPinConfig;

typedef struct {
  uint8_t id;
  UARTDriver *driver;
  DMXUartPin uart_pad;
  DMXPin dir_pad;
  DMXPin ledout_pad;
  DMXPin ledin_pad;
} DMXConfig;

typedef enum
{
  DMX_IN,
  DMX_OUT
} eDmxDirection;

typedef enum
{
  BREAK,
  IDLE
} eDmxState;

void dmxInit(DMXConfig *cfg);
void dmxStart(DMXConfig *cfg);
void dmxStop(DMXConfig *cfg);
void dmxSetDirection(DMXConfig *cfg, eDmxDirection dir);
void dmxSetChannel(uint8_t port, uint16_t channel, uint8_t value);

uint8_t dmxUpdate(uint8_t *data, uint8_t len);
uint8_t dmxSetStream(uint8_t *data, uint8_t len, uint8_t start);
void dmxGetStream(uint8_t port, uint8_t *data, uint8_t len, uint8_t start);
uint8_t dmxGetChannel(uint8_t port, uint16_t channel);

#endif
