
#include "config.h"
#include "eeprom.h"

// Default Values

DeviceConfig gDeviceConfig;

void configLoad(void)
{
  // Initialize device config with default values
  gDeviceConfig.port[0] = dmxGetPortConfig(0);
  gDeviceConfig.port[1] = dmxGetPortConfig(1);
  gDeviceConfig.port[2] = dmxGetPortConfig(2);
  
  
}

DMXPortConfig configGetPortConfig(uint8_t port)
{
  return dmxGetPortConfig(port);
}

bool configSetPortConfig(uint8_t port, DMXPortConfig *cfg)
{
  if(port > 2)
    return false;

  bool ret = dmxSetPortConfig(port, cfg);

  if(ret)
  {
    // Save configuration values to eeprom
    gDeviceConfig.port[port] = dmxGetPortConfig(port);
    
    return true;
  }

  return false;
}
