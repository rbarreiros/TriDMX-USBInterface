#!/bin/bash
if [ "$(uname)" == "Darwin" ]; then
st-flash --reset write ../bootloader/usbdfu_text.bin 0x8000000
else
../../stlink/st-flash --reset write ../bootloader/usbdfu_text.bin 0x8000000
fi
