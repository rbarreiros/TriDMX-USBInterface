#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>

// Port Modes

#define DIRECTION_OUTPUT 0x00
#define DIRECTION_INPUT  0x01
#define DIRECTION_MERGE  0x02
#define DIRECTION_MIRROR 0x03

#define MERGE_LTP 0x00
#define MERGE_HTP 0x01

#define SOURCE_0   0x00
#define SOURCE_1   0x01
#define SOURCE_2   0x02
#define SOURCE_USB 0x03

typedef struct
{
  uint8_t id;             // port id
  uint8_t direction;      // direction:
                          // 0x00 Output,
                          // 0x01 Input
                          // 0x02 Merge Output
                          // 0x03 Mirror
  uint8_t merge_source_a; // merge source a - port id 0,1,2 (except self ofc) or 3 - USB
  uint8_t merge_source_b; // merge source b
  uint8_t merge_htp_ltp;  // merge method, 0x00 - htp, 0x01 ltp
  uint32_t stream_ts;     // Timestamp (system tick) of the last dmxStream update (sent or received)
} DMXPortConfig;

typedef struct
{
  DMXPortConfig port0;
  DMXPortConfig port1;
  DMXPortConfig port2;
  // ... Other future options
} DeviceConfig;

#endif
