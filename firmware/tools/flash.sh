#!/bin/bash
if [ "$(uname)" == "Darwin" ]; then
st-flash --reset write build/ch.bin 0x8000000
else
../stlink/st-flash --reset write build/ch.bin 0x8000000
fi
