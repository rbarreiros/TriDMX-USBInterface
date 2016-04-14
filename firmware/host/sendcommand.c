
#include <stdio.h>
#include <libusb-1.0/libusb.h>
#include <string.h>
#include <stdlib.h>

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

void print_help(char *str)
{
  printf("Usage: %s [command] [data]\n\n", str);
}

int main(int argc, char **argv)
{
  int r = 1;  // result
  int i, n;
  uint8_t data[60];
  unsigned int timeout = 25;
  
  if(argc < 3)
  {
    print_help(argv[0]);
    return 0;
  }

  uint8_t command = atoi(argv[1]);

  for(i = 2; i < argc; i++)
  {
    if(i > 59)
      break;

    data[i-2] = atoi(argv[i]);
  }

  printf("Command 0x%02x\n", command);

  for(i = 0; i < (argc - 2); i++)
    printf("Data 0x%02x\n", data[i]);
  printf("\n");
  
  //init libUSB

  r = libusb_init(&ctx);
  if (r < 0) {
    fprintf(stderr, "Failed to initialise libusb\n");
    return 1;
  }

  //libusb_set_debug(ctx, 3);

  //open the device
  devh = libusb_open_device_with_vid_pid(ctx,
                                         USB_VENDOR_ID, USB_PRODUCT_ID);
  if (!devh) {
    perror("device not found");
    return 1;
  }

  /*
  r = libusb_set_configuration(devh, 1);
  if(r < 0) {
    fprintf(stderr, "usb_set_configuration error %d\n", r);
    goto clean_out;
  }
  */
  
  //claim the interface
  r = libusb_claim_interface(devh, 0);
  if (r < 0) {
    fprintf(stderr, "usb_claim_interface error %d\n", r);
    exitflag = out;
  } else  {
    printf("Claimed interface\n");
    uint8_t tmp[64];

    tmp[0] = command;
    tmp[1] = (argc - 2) + 2;
    memcpy(&tmp[2], data, (argc - 2));

    r = libusb_bulk_transfer(devh, USB_ENDPOINT_OUT, tmp, (argc - 2) + 2, &n, timeout);

    if(r == 0)
    {
      printf("sent %d bytes to device\n", n);
      for(int i = 0; i < n; i++)
        printf("0x%02x ", tmp[i]);
      printf("\n");
    }
    else
      printf("ERROR in bulk write: %d %s\n", r, libusb_error_name(r));
    
    uint8_t buff[512] = {0};
    int nread;
    r = libusb_bulk_transfer(devh, USB_ENDPOINT_IN, buff, sizeof(buff), &nread, 30);
    if(r == 0)
    {
      printf("Read %d\n", nread);
      for(int i = 0; i < nread; i++)
        printf("0x%02x ", buff[i]);
      printf("\n");
    }
  }
  
clean_out:
  libusb_release_interface(devh, 0);
  libusb_close(devh);
  libusb_exit(NULL);

  return 0;
}
