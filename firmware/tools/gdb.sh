#!/bin/bash
arm-none-eabi-gdb --eval-command="target remote :3333" build/ch.elf
