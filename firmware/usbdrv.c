#include <ch.h>
#include <hal.h>
#include <serial_usb.h>

// TODO - Change the VID/PID to proper values 

#define USB_VID 0x0483
#define USB_PID 0xFEDC

// Location of STM32F1XX device unique ID

#define         ID1          (0x1FFFF7E8)
#define         ID2          (0x1FFFF7EC)
#define         ID3          (0x1FFFF7F0)

//#define USB_VID 0x2012
//#define USB_PID 0x1029

/* Virtual serial port over USB.*/
SerialUSBDriver SDU1;

/*
 * Endpoints to be used for USBD1.
 */
#define USBD1_DATA_REQUEST_EP           1
#define USBD1_DATA_AVAILABLE_EP         1

//////////////////////////////////////// 

bool gDoShutdown = 0;

struct usb_request {
  uint8_t  bmRequestType;
  uint8_t  bRequest;
  uint16_t wValue;
  uint16_t wIndex;
  uint16_t wLength;
};

////////////////////////////////////////

static void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len);

/*
 * USB Device Descriptor.
 */
static const uint8_t vcom_device_descriptor_data[18] = {
  USB_DESC_DEVICE       (0x0200,        /* bcdUSB (1.1).                    */
                         0x00,          /* bDeviceClass (CDC).              */
                         0x00,          /* bDeviceSubClass.                 */
                         0x00,          /* bDeviceProtocol.                 */
                         0x40,          /* bMaxPacketSize.                  */
                         USB_VID,       /* idVendor (ST).                   */
                         USB_PID,       /* idProduct.                       */
                         0x0200,        /* bcdDevice.                       */
                         1,             /* iManufacturer.                   */
                         2,             /* iProduct.                        */
                         3,             /* iSerialNumber.                   */
                         1)             /* bNumConfigurations.              */
};

/*
 * Device Descriptor wrapper.
 */
static const USBDescriptor vcom_device_descriptor = {
  sizeof vcom_device_descriptor_data,
    vcom_device_descriptor_data
};

static const uint8_t vcom_configuration_descriptor_data[32] = {
  /* Configuration Descriptor.*/
  USB_DESC_CONFIGURATION(32,            /* wTotalLength.                    */
                         0x01,          /* bNumInterfaces.                  */
                         0x01,          /* bConfigurationValue.             */
                         0,             /* iConfiguration.                  */
                         0x80,          /* bmAttributes (self powered).     */
                         50),           /* bMaxPower (100mA).               */
  /* Interface Descriptor.*/
  USB_DESC_INTERFACE    (0x00,          /* bInterfaceNumber.                */
                         0x00,          /* bAlternateSetting.               */
                         0x02,          /* bNumEndpoints.                   */
                         0xFF,          /* bInterfaceClass (Data Class      */
                         0x00,          /* bInterfaceSubClass (CDC section 4.6). */
                         0x00,          /* bInterfaceProtocol (CDC section 4.7). */
                         0x00),         /* iInterface.                      */
  /* Endpoint 3 Descriptor.*/
  USB_DESC_ENDPOINT     (USBD1_DATA_AVAILABLE_EP,       /* bEndpointAddress.*/
                         0x02,          /* bmAttributes (Bulk).             */
                         0x0040,        /* wMaxPacketSize.                  */
                         0x00),         /* bInterval.                       */
  /* Endpoint 1 Descriptor.*/
  USB_DESC_ENDPOINT     (USBD1_DATA_REQUEST_EP|0x80,    /* bEndpointAddress.*/
                         0x02,          /* bmAttributes (Bulk).             */
                         0x0040,        /* wMaxPacketSize.                  */
                         0x00)          /* bInterval.                       */
};

/*
 * Configuration Descriptor wrapper.
 */
static const USBDescriptor vcom_configuration_descriptor = {
  sizeof vcom_configuration_descriptor_data,
    vcom_configuration_descriptor_data
};

/*
 * U.S. English language identifier.
 */
static const uint8_t vcom_string0[] = {
  USB_DESC_BYTE(4),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  USB_DESC_WORD(0x0409)                 /* wLANGID (U.S. English).          */
};

/*
 * Vendor string.
 */
static const uint8_t vcom_string1[] = {
  USB_DESC_BYTE(28),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'R', 0,
  'u', 0,
  'i', 0,
  ' ', 0,
  'B', 0,
  'a', 0,
  'r', 0,
  'r', 0,
  'e', 0,
  'i', 0,
  'r', 0,
  'o', 0,
  's', 0
};

/*
 * Device Description string.
 */
static const uint8_t vcom_string2[] = {
  USB_DESC_BYTE(64),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'T', 0,
  'r', 0,
  'i', 0,
  'D', 0,
  'M', 0,
  'X', 0,
  ' ', 0,
  '3', 0,
  ' ', 0,
  'P', 0,
  'o', 0,
  'r', 0,
  't', 0,
  ' ', 0,
  'D', 0,
  'M', 0,
  'X', 0,
  ' ', 0,
  'U', 0,
  'S', 0,
  'B', 0,
  ' ', 0,
  'I', 0,
  'n', 0,
  't', 0,
  'e', 0,
  'r', 0,
  'f', 0,
  'a', 0,
  'c', 0,
  'e', 0
};

/*
 * Serial Number string.
 */
//static const uint8_t vcom_string3[] = {
static uint8_t vcom_string3[] = {
  USB_DESC_BYTE(28), // bLength.
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING),     // bDescriptorType.
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
 * Strings wrappers array.
 */

static const USBDescriptor vcom_strings[] = {
  {sizeof vcom_string0, vcom_string0},
  {sizeof vcom_string1, vcom_string1},
  {sizeof vcom_string2, vcom_string2},
  {sizeof vcom_string3, vcom_string3}
};
  
/*
 * Handles the GET_DESCRIPTOR callback. All required descriptors must be
 * handled here.
 */
static const USBDescriptor *get_descriptor(USBDriver *usbp,
                                           uint8_t dtype,
                                           uint8_t dindex,
                                           uint16_t lang)
{
  (void)usbp;
  (void)lang;
  switch (dtype) {
    case USB_DESCRIPTOR_DEVICE:
      return &vcom_device_descriptor;
    case USB_DESCRIPTOR_CONFIGURATION:
      return &vcom_configuration_descriptor;
    case USB_DESCRIPTOR_STRING:
      if (dindex < 4)
        return &vcom_strings[dindex];
  }
  return NULL;
}

/**
 * @brief   IN EP1 state.
 */
static USBInEndpointState ep1instate;

/**
 * @brief   OUT EP1 state.
 */
static USBOutEndpointState ep1outstate;

/**
 * @brief   EP1 initialization structure (both IN and OUT).
 */
static const USBEndpointConfig ep1config = {
  USB_EP_MODE_TYPE_BULK,
  NULL,
  sduDataTransmitted,
  sduDataReceived,
  0x0040,
  0x0040,
  &ep1instate,
  &ep1outstate,
  2,
  NULL
};

/*
 * Handles the USB driver global events.
 */
static void usb_event(USBDriver *usbp, usbevent_t event) {
  extern SerialUSBDriver SDU1;

  switch (event) {
    case USB_EVENT_RESET:
      return;
    case USB_EVENT_ADDRESS:
      return;
    case USB_EVENT_CONFIGURED:
      chSysLockFromISR();
      /* Enables the endpoints specified into the configuration.
       Note, this callback is invoked from an ISR so I-Class functions
       must be used.*/
      usbInitEndpointI(usbp, USBD1_DATA_REQUEST_EP, &ep1config);
      /* Resetting the state of the CDC subsystem.*/
      sduConfigureHookI(&SDU1);
      chSysUnlockFromISR();
      return;
    case USB_EVENT_SUSPEND:
      chSysLockFromISR();
      /* Disconnection event on suspend.*/
      sduDisconnectI(&SDU1);
      chSysUnlockFromISR();
      return;
    case USB_EVENT_WAKEUP:
      return;
    case USB_EVENT_STALLED:
      return;
  }
  return;
}

/*
 * Handles the USB driver global events.
 */
static void sof_handler(USBDriver *usbp) {

  (void)usbp;

  osalSysLockFromISR();
  sduSOFHookI(&SDU1);
  osalSysUnlockFromISR();
}

// Handle the device reset on USB to enter bootloader
/*
static bool request_hook(USBDriver *usbp)
{
  struct usb_request *rq;

  rq = (struct usb_request *)usbp->setup;

  if ((rq->bmRequestType & USB_RTYPE_TYPE_MASK) != USB_RTYPE_TYPE_VENDOR)
    return sduRequestsHook(usbp);

  switch (rq->wIndex) {
    case 0:
      gDoShutdown = 1;
      return true;
    default:
      return false;
  }

  return false;
}
*/

/*
 * USB driver configuration.
 */
const USBConfig usbcfg = {
  usb_event,
  get_descriptor,
  //request_hook,
  sduRequestsHook,
  sof_handler
};

/*
 * Serial over USB driver configuration.
 */
const SerialUSBConfig serusbcfg = {
  &USBD1,
  USBD1_DATA_REQUEST_EP,
  USBD1_DATA_AVAILABLE_EP,
  0
};

// Function copied from STM32_USB_FS_Device_Lib

/*******************************************************************************
 * Function Name  : Get_SerialNum.
 * Description    : Create the serial number string descriptor.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Get_SerialNum(void)
{
  uint32_t Device_Serial0, Device_Serial1, Device_Serial2;

  Device_Serial0 = *(uint32_t*)ID1;
  Device_Serial1 = *(uint32_t*)ID2;
  Device_Serial2 = *(uint32_t*)ID3;

  Device_Serial0 += Device_Serial2;

  if (Device_Serial0 != 0)
  {
    IntToUnicode (Device_Serial0, &vcom_string3[2] , 8);
    IntToUnicode (Device_Serial1, &vcom_string3[18], 4);
  }
}

/*******************************************************************************
 * Function Name  : HexToChar.
 * Description    : Convert Hex 32Bits value into char.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
static void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len)
{
  uint8_t idx = 0;

  for( idx = 0 ; idx < len ; idx ++)
  {
    if( ((value >> 28)) < 0xA )
    {
      pbuf[ 2* idx] = (value >> 28) + '0';
    }
    else
    {
      pbuf[2* idx] = (value >> 28) + 'A' - 10;
    }

    value = value << 4;

    pbuf[ 2* idx + 1] = 0;
  }
}
