#
# Copyright 2012 Fabio Baltieri (fabio.baltieri@gmail.com)
#
# Based on the original ben-wpan code written by:
#   Werner Almesberger, Copyright 2010-2011
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#

NAME = usbdfu

CPU = cortex-m3
MCU = STM32F1

# this needs libopencm and a cross-compiler with newlib support
LIBOPENCM3 = ../../libopencm3
LDSCRIPT = stm32-f103.ld

CROSS_COMPILE = arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
SIZE = $(CROSS_COMPILE)size

#BOOTLOADER = dfu-util
#BOOTLOADER_FLAGS = -a 0 --dfuse-address 0x08000000:leave

BOOTLOADER = ../../stlink/st-flash
BOOTLOADER_FLAGS = --reset write $(NAME)_text.bin 0x8000000

CFLAGS  = -Os -g3 -Wall -Wextra -Wimplicit-function-declaration -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes -Wundef -Wshadow
CFLAGS += -I$(LIBOPENCM3)/include -fno-common -mthumb -mcpu=$(CPU) -msoft-float -MD -D$(MCU) --data-sections --function-sections
ASFLAGS =
LDFLAGS  = -lopencm3_stm32f1 --static -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group -L$(LIBOPENCM3)/lib -T$(LDSCRIPT) -nostartfiles
LDFLAGS += -Wl,--gc-sections -mthumb -mcpu=cortex-m3 -msoft-float -mfix-cortex-m3-ldrd


OBJS = usbdfu.o

# ----- Verbosity control -----------------------------------------------------

CC_normal	:= $(CC)
BUILD_normal	:=
DEPEND_normal	:= $(CPP) $(CFLAGS) -MM -MG

CC_quiet	= @echo "  CC       " $@ && $(CC_normal)
BUILD_quiet	= @echo "  BUILD    " $@ && $(BUILD_normal)
DEPEND_quiet	= @$(DEPEND_normal)

ifeq ($(V),1)
    CC		= $(CC_normal)
    BUILD	= $(BUILD_normal)
    DEPEND	= $(DEPEND_normal)
else
    CC		= $(CC_quiet)
    BUILD	= $(BUILD_quiet)
    DEPEND	= $(DEPEND_quiet)
endif

# ----- Rules -----------------------------------------------------------------

.PHONY:	all clean

all: $(NAME)_text.bin $(NAME)_text.hex

$(NAME).elf: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)
	$(SIZE) $@

%_text.bin: %.elf
	$(BUILD) $(OBJCOPY) -j .text -j .data -O binary $< $@

%_text.hex: %.elf
	$(BUILD) $(OBJCOPY) -j .text -j .data -O ihex $< $@

# ----- Cleanup ---------------------------------------------------------------

clean:
	rm -f $(NAME).bin $(NAME).elf $(NAME).hex
	rm -f $(NAME)_text.bin $(NAME)_text.hex
	rm -f $(OBJS) $(OBJS:.o=.d)
	rm -f *~

# ----- Dependencies ----------------------------------------------------------

#MKDEP =	\
#	$(DEPEND) $< | \
#	  sed \
#	    -e 's|^$(basename $(notdir $<)).o:|$@:|' \
#	    -e '/^\(.*:\)\? */{p;s///;s/ *\\\?$$/ /;s/  */:\n/g;H;}' \
#	    -e '$${g;p;}' \
#	    -e d >$(basename $@).d; \
#	  [ "$${PIPESTATUS[*]}" = "0 0" ] || \
#	  { rm -f $(basename $@).d; exit 1; }

%.o: %.c
	$(CC) $(CFLAGS) -Os -c $< -o $@
	$(MKDEP)

%.o: %.S
	$(CC) $(CFLAGS) -D__ASSEMBLY__ -Os -c $< -o $@
	$(MKDEP)

-include $(OBJS:.o=.d)

# ----- Programming and device control ----------------------------------------

.PHONY: load fuse boot

boot: $(NAME)_text.bin
	$(BOOTLOADER) $(BOOTLOADER_FLAGS)
