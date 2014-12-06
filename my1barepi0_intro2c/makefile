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

CFLAGS += -O2 -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s
CFLAGS += -nostdlib -nostartfiles -ffreestanding

pi: main.img

clean:
	rm -rf *.img *.lst *.map

%.img: %.elf
	$(TOOLPFIX)objcopy $< -O binary $@
	$(TOOLPFIX)objcopy $< -O binary $(TARGET)

%.elf: %.o $(LINKER)
	$(TOOLPFIX)ld --no-undefined $< -Map $(MAP) -o $@ -T $(LINKER)
	$(TOOLPFIX)objdump -d $@ > $(LST)

%.o: %.c
	$(TOOLPFIX)gcc $(CFLAGS) -c $< -o $@
