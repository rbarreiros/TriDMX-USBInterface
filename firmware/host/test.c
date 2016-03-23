
#include <stdio.h>
#include <libusb-1.0/libusb.h>
#include <string.h>

#define USB_VENDOR_ID  0x0483
#define USB_PRODUCT_ID 0xFEDC
#define USB_ENDPOINT_IN  (LIBUSB_ENDPOINT_IN  | 1)   /* endpoint address */
#define USB_ENDPOINT_OUT (LIBUSB_ENDPOINT_OUT | 1)   /* endpoint address */

//Global variables:
struct libusb_device_handle *devh = NULL;
static libusb_context *ctx = NULL;

enum {
  out_deinit,
  out_release,
  out
} exitflag;

int main(void)
{
  int r = 1;  // result
  int i;

  //init libUSB

  r = libusb_init(&ctx);
  if (r < 0) {
    fprintf(stderr, "Failed to initialise libusb\n");
    return 1;
  }

  libusb_set_debug(ctx, LIBUSB_LOG_LEVEL_DEBUG);

  //open the device
  devh = libusb_open_device_with_vid_pid(ctx,
                                         USB_VENDOR_ID, USB_PRODUCT_ID);
  if (!devh) {
    perror("device not found");
    return 1;
  }

  r = libusb_set_configuration(devh, 1);
  if(r < 0) {
    fprintf(stderr, "usb_claim_interface error %d\n", r);
    goto clean_out;
  }
  
  //claim the interface
  r = libusb_claim_interface(devh, 0);
  if (r < 0) {
    fprintf(stderr, "usb_claim_interface error %d\n", r);
    exitflag = out;
  } else  {
    printf("Claimed interface\n");
    int n = 0;
    int port = 2;
    unsigned char data[512] = {0};
    uint8_t started = 0;
    
    for(int i = 0, j = 1; i < sizeof(data); i++, j++)
    {
      if(j > 255) j = 1;
      data[i] = j;
    }

    int sizeLeft = sizeof(data);
    while(sizeLeft)
    {
      if(sizeLeft > 60)
      {
        unsigned char tmp[63] = {0};

        tmp[0] = (started == 0) ? 0x11 : 0x12;
        tmp[1] = sizeof(tmp);
        tmp[2] = port;
        
        int start = sizeof(data) - sizeLeft;
        memcpy( &tmp[3], &data[start], 60);
        r = libusb_bulk_transfer(devh, USB_ENDPOINT_OUT, tmp, sizeof(tmp), &n, 25);        
        sizeLeft -= 60;
        started = 1;
        
        for(int i = 0; i < sizeof(tmp); i++)
          printf("0x%02x ", tmp[i]);
        printf("\n");
        
      } else {
        unsigned char tmp[sizeLeft + 3];
        memset(tmp, 0, sizeof(tmp));

        tmp[0] = (started == 0) ? 0x11 : 0x12;
        tmp[1] = sizeof(tmp);
        tmp[2] = port;

        int start = sizeof(data) - sizeLeft;
        memcpy(&tmp[3], &data[start], sizeLeft);
        r = libusb_bulk_transfer(devh, USB_ENDPOINT_OUT, tmp, sizeof(tmp), &n, 25);
        sizeLeft = 0;
        started = 0;
        
        for(int i = 0; i < sizeof(tmp); i++)
          printf("0x%02x ", tmp[i]);
        printf("\n");
      }
      
      switch(r){
        case 0:
          printf("send %d bytes to device\n", n);
          //for(int i = 0; i < sizeof(tmp); i++)
          //printf("0x%02x ", tmp[i]);
          //printf("\n");
          break;
        case LIBUSB_ERROR_TIMEOUT:
          printf("ERROR in bulk write: %d Timeout\n", r);
          break;
        case LIBUSB_ERROR_PIPE:
          printf("ERROR in bulk write: %d Pipe\n", r);
          break;
        case LIBUSB_ERROR_OVERFLOW:
          printf("ERROR in bulk write: %d Overflow\n", r);
          break;
        case LIBUSB_ERROR_NO_DEVICE:
          printf("ERROR in bulk write: %d No Device\n", r);
          break;
        default:
          printf("ERROR in bulk write: %d\n", r);
          break;
      }
      
      uint8_t buff[1] = {0};
      r = libusb_bulk_transfer(devh, USB_ENDPOINT_IN, buff, sizeof(buff), &n, 25);
      if(r == 0)
      {
        for(int i = 0; i < sizeof(buff); i++)
          printf("0x%02x ", buff[i]);
        printf("\n");
      }
      
    }

  }

clean_out:
  libusb_release_interface(devh, 0);
  libusb_close(devh);
  libusb_exit(NULL);

  return 0;
}
