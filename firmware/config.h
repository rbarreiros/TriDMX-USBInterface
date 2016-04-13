#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>

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
} DMXPortConfig;

typedef struct
{
  DMXPortConfig port0;
  DMXPortConfig port1;
  DMXPortConfig port2;
  // ... Other future options
} DeviceConfig;

#endif
