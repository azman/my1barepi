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

pi: main.img

clean:
	rm -rf *.img *.lst *.map

new: clean pi

%.img: %.elf
	$(TOOLPFIX)objcopy $< -O binary $@
	$(TOOLPFIX)objcopy $< -O binary $(TARGET)

%.elf: %.o $(LINKER)
	$(TOOLPFIX)ld --no-undefined $< -Map $(MAP) -o $@ -T $(LINKER)
	$(TOOLPFIX)objdump -d $@ > $(LST)

%.o: %.s
	$(TOOLPFIX)as $< -o $@
