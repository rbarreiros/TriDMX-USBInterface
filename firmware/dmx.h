#ifndef __DMX_H__
#define __DMX_H__

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

#define DMX_BUFFER_SIZE 513
#define DMX_BAUDRATE    250000
#define BREAK_BAUDRATE  90000

#define DMX1_GPIO_BANK GPIOC
#define DMX1_LED       13

#define DMX1_DIR_GPIO  GPIOC
#define DMX1_DIR_PIN   1

void dmx1Init(void);
void dmx2Init(void);
void dmx3Init(void);

void dmx1Start(void);
void dmx2Start(void);
void dmx3Start(void);

void dmx1Stop(void);
void dmx2Stop(void);
void dmx3Stop(void);

void dmx1SetDirection(eDmxDirection dir);
void dmx2SetDirection(eDmxDirection dir);
void dmx3SetDirection(eDmxDirection dir);

void dmx1SetChannel(uint16_t channel, uint8_t value);
void dmx2SetChannel(uint16_t channel, uint8_t value);
void dmx3SetChannel(uint16_t channel, uint8_t value);

#endif