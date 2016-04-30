# to generate raspberry pi bare-metal code (kernel.img)

ifeq ($(OS),Windows_NT)
TOOLPATH ?= /c/users/public/tool/xtool-arm/bin/
else
TOOLPATH ?= /home/share/tool/xtool-arm/bin/
endif
TOOLPFIX ?= $(TOOLPATH)arm-none-eabi-

LINKER = kernel.ld
TARGET = kernel.img
LST = kernel.lst
MAP = kernel.map

CFLAGS += -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s
CFLAGS += -nostdlib -nostartfiles -ffreestanding -Wall
LFLAGS += --no-undefined

help:
	@echo "Targets: help clean pi <filename>.img"

pi: main.img

clean:
	rm -rf *.img *.lst *.map
# *.elf *.o deleted by compiler once done with

new: clean pi

%.img: %.elf
	$(TOOLPFIX)objcopy $< -O binary $@
	$(TOOLPFIX)objcopy $< -O binary $(TARGET)

%.elf: %.o $(LINKER)
	$(TOOLPFIX)ld $(LFLAGS) $< -Map $(MAP) -o $@ -T $(LINKER)
	$(TOOLPFIX)objdump -d $@ > $(LST)

%.o: %.c
	$(TOOLPFIX)gcc $(CFLAGS) -c $< -o $@
