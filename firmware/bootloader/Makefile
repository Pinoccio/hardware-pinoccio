# Makefile to compile the pinoccio bootloader
#
# Copyright 2014 Matthijs Kooijman <matthijs@stdin.nl>
#
# Permission is hereby granted, free of charge, to anyone obtaining a
# copy of this document to do whatever they want with them without any
# restriction, including, but not limited to, copying, modification and
# redistribution.
#
# NO WARRANTY OF ANY KIND IS PROVIDED.
#
# To compile, just make sure that avr-gcc and friends are in your path
# and type "make".
#
PRG            = bootloader
OBJ            = src/main.o src/wibo.o
MCU            = atmega256rfr2
URACOLI        = uracoli-src-20131127
DEFS           = -DF_CPU=16000000UL -D_PINOCCIO_256RFR2_=1 -Dpinoccio -D_SW_VERSION_=5 -DWIBO_FLAVOUR_BOOTLUP=1
INCLUDES       = -I$(URACOLI)/inc -I$(URACOLI)/inc/boards
OPTIMIZE       = -Os
LIBS           = -luracoli_pinoccio -L $(URACOLI)/lib

CFLAGS        = $(DEFS) $(INCLUDES) $(OPTIMIZE)
CFLAGS       += -fdata-sections -fpack-struct -fshort-enums -g3 -Wall -pedantic -mmcu=$(MCU)
CFLAGS       += -std=gnu99 -fno-strict-aliasing -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith -mrelax
# Workaround until https://gcc.gnu.org/git/?p=gcc.git;a=commit;h=986b9a67c847f59e5c383e1f8d6faa7431a11911 is released
CFLAGS       += -fno-jump-tables
LDFLAGS       = -Wl,-Map,$(PRG).map -Wl,--gc-sections -Wl,--section-start=.text=0x3E000 -Wl,--section-start=.bootlup=0x3FD00
LDFLAGS      += -mmcu=$(MCU) -Wl,--relax

CC             = avr-gcc
OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump


all: hex

lst:  $(PRG).lst

hex:  $(PRG).hex

clean: uracoli_clean
	rm -rf $(OBJ) $(OBJ:.o=.d) $(PRG).elf $(PRG).hex $(PRG).lst $(PRG).map

$(PRG).elf: $(OBJ) uracoli
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

uracoli:
	$(MAKE) -C $(URACOLI)/src pinoccio

uracoli_clean:
	$(MAKE) -C $(URACOLI)/src clean
	# uracoli forgets to clean its actual build result
	rm -rf $(URACOLI)/lib

.PHONY: uracoli all lst hex clean uracoli_clean

# pull in dependency info for *existing* .o files
-include $(OBJ:.o=.d)
