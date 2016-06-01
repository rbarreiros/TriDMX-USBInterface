#ifndef __DMX_H__
#define __DMX_H__

#include <stdint.h>

#define DMX_THREAD_SIZE 256

#define DMX_BUFFER_SIZE 513
#define DMX_BAUDRATE    250000 // 375000
#define BREAK_BAUDRATE  75000  // 110000

// Port Modes

#define MERGE_LTP 0x00
#define MERGE_HTP 0x01

#define SOURCE_0   0x00
#define SOURCE_1   0x01
#define SOURCE_2   0x02

// DMX Direction
typedef enum
{
  DIRECTION_OUTPUT = 0,
  DIRECTION_INPUT,
  DIRECTION_MERGE,
  DIRECTION_MIRROR,
  DIRECTION_MAX
} eDmxDirection;

// Each Port configuration and status
typedef struct
{
  eDmxDirection direction; // direction:
                           // 0x00 Output,
                           // 0x01 Input
                           // 0x02 Merge Output
                           // 0x03 Mirror
  uint8_t merge_source_a;  // merge source a - port id 0,1,2 (if self it's usb self)
  uint8_t merge_source_b;  // merge source b
  uint8_t merge_htp_ltp;   // merge method, 0x00 - htp, 0x01 ltp
} DMXPortConfig;

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
  uint8_t id;
  UARTDriver *driver;
  DMXUartPin uart_pad;
  DMXPin dir_pad;
  DMXPin ledout_pad;
  DMXPin ledin_pad;
  DMXPortConfig cfg;
  bool id_enabled;
  bool started;
  uint32_t stream_ts;      // Timestamp (system tick) of the last dmxStream update (sent or received)
} DMXConfig;

typedef enum
{
  BREAK,
  IDLE
} eDmxState;

void dmxInit(uint8_t id);
void dmxStart(uint8_t id);
void dmxStop(uint8_t id);
bool dmxSetDirection(uint8_t id, eDmxDirection dir);
void dmxSetChannel(uint8_t port, uint16_t channel, uint8_t value);

uint8_t dmxUpdate(uint8_t port, uint8_t *data, uint8_t len);
uint8_t dmxSetStream(uint8_t port, uint8_t *data, uint8_t len, uint8_t start);
uint8_t dmxGetStream(uint8_t port, uint8_t *data, uint8_t len);
uint8_t dmxGetChannel(uint8_t port, uint16_t channel);
void dmxIdentify(uint8_t port);
DMXPortConfig dmxGetPortConfig(uint8_t port);
bool dmxSetPortConfig(uint8_t port, DMXPortConfig *cfg);

extern THD_WORKING_AREA(waDMX, DMX_THREAD_SIZE);
THD_FUNCTION(DMXThread, arg);

#endif
