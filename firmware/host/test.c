
#include <stdio.h>
//#include <libusb-1.0/libusb.h>

#define USB_VENDOR_ID  0x0483
#define USB_PRODUCT_ID 0xFEDC
#define USB_ENDPOINT_IN  (LIBUSB_ENDPOINT_IN  | 1)   /* endpoint address */
#define USB_ENDPOINT_OUT (LIBUSB_ENDPOINT_OUT | 1)   /* endpoint address */

//Global variables:
//struct libusb_device_handle *devh = NULL;
//static libusb_context *ctx = NULL;

enum {
  out_deinit,
  out_release,
  out
} exitflag;

int main(void)
{
  int r = 1;  // result
  int i;

  printf("DEBUG\n");

  //init libUSB
  /*
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

  //claim the interface
  r = libusb_claim_interface(devh, 0);
  if (r < 0) {
    fprintf(stderr, "usb_claim_interface error %d\n", r);
    exitflag = out;
  } else  {
    printf("Claimed interface\n");
    int n = 0;
    unsigned char tmp[5] = {0xAF, 0xEB, 0xA5, 0x05, 0x44};
    r = libusb_bulk_transfer(devh, USB_ENDPOINT_OUT, &tmp, 5, &n, 3000);
    switch(r){
      case 0:
        printf("send %d bytes to device\n", n);
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

    uint8_t buff[64] = {0};
    r = libusb_bulk_transfer(devh, USB_ENDPOINT_IN, &buff, 64, &n, 3000);
    if(r != 0)
    {
      for(int i = 0; i < 64; i++)
      {
        printf("%02x", buff[i]);
      }
      printf("\n");
    }
  }

  libusb_release_interface(devh, 0);
  libusb_close(devh);
  libusb_exit(NULL);
*/
  return 0;
}
