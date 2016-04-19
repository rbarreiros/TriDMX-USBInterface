#ifndef __USBDRV_H__
#define __USBDRV_H__

#define USB_MAX_PACKET_SIZE   64

extern const USBConfig usbcfg;
extern SerialUSBConfig serusbcfg;
extern SerialUSBDriver SDU1;

extern bool gDoShutdown;

void Get_SerialNum(void);

#endif
