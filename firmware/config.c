
#include "config.h"
#include "eeprom.h"
#include "stm32f10x_flash.h"

/* Virtual address defined by the user: 0xFFFF value is prohibited */
uint16_t VirtAddVarTab[NumbOfVar];

// Default Values
DeviceConfig gDeviceConfig;


void configSave(void)
{
  int size = sizeof(DeviceConfig)/2;
  int i = 0;
  uint16_t *buff = (uint16_t*)&gDeviceConfig;
  int ret = 0;
  
  for(i = 0; i < size; i++)
  {
    ret = EE_WriteVariable(i, buff[i]);

    if(ret != FLASH_COMPLETE)
    {
      // An error ocurred, figure it out
      return;
    }
  }
}

void configLoad(void)
{
  int ret = 0;
  int size = sizeof(DeviceConfig)/2;
  int i = 0;
  uint16_t *buff = (uint16_t*)&gDeviceConfig;

  // Initialize EEPROM subsystem
  EE_Init();

  // Read config
  for(i = 0; i < size; i++)
  {
    // Init the virtual address table at the same time
    VirtAddVarTab[i] = i;
    
    ret = EE_ReadVariable(i, &buff[i]);

    if(ret != 0)
    {
      // Variable not found, load defaults
      // Initialize device config with default values
      gDeviceConfig.port[0] = dmxGetPortConfig(0);
      gDeviceConfig.port[1] = dmxGetPortConfig(1);
      gDeviceConfig.port[2] = dmxGetPortConfig(2);
      // Save it!
      configSave();
      return;
    }
  }
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
    configSave();
    
    return true;
  }

  return false;
}
