# to generate raspberry pi bare-metal code (kernel.img)

ifeq ($(OS),Windows_NT)
TOOLPATH ?= /c/users/public/tool/xtool-arm/bin/
else
TOOLPATH ?= /home/share/tool/xtool-arm/bin/
endif
TOOLPFIX ?= $(TOOLPATH)arm-none-eabi

# multiple source files (list them here as object files!)
OBJLST = boot.o gpio.o timer.o uart.o utils.o main.o

LINKER = kernel.ld
TARGET = kernel.img
LST = kernel.lst
MAP = kernel.map

AFLAGS +=
CFLAGS += -O2 -mfpu=vfp -mfloat-abi=hard
CFLAGS += -nostdlib -nostartfiles -ffreestanding
CFLAGS += -march=armv6zk -mtune=arm1176jzf-s
CFLAGS += -I../my1barepi4_uartmod/src/
CFLAGS += -I../my1barepi2_timermod/src/
LFLAGS += --no-undefined -L/usr/lib/gcc/i486-slackware-linux/4.8.2/libgcc.a

# prevent make from automatically removing these!
.SECONDARY: $(OBJLST)

all: $(TARGET)

clean:
	rm -rf *.img *.elf *.lst *.map *.o

new: clean all

$(TARGET): main.elf
	$(TOOLPFIX)-objcopy $< -O binary $@

%.elf: $(OBJLST)
	$(TOOLPFIX)-ld -o $@ $^ -Map $(MAP) -T $(LINKER) $(LFLAGS)
	$(TOOLPFIX)-objdump -d $@ > $(LST)

%.o: src/%.s
	$(TOOLPFIX)-as $(AFLAGS) $< -o $@

%.o: src/%.c src/%.h
	$(TOOLPFIX)-gcc $(CFLAGS) -c $< -o $@

%.o: src/%.c
	$(TOOLPFIX)-gcc $(CFLAGS) -c $< -o $@

%.o: ../my1barepi4_uartmod/src/%.c ../my1barepi4_uartmod/src/%.h
	$(TOOLPFIX)-gcc $(CFLAGS) -c $< -o $@

%.o: ../my1barepi2_timermod/src/%.c ../my1barepi2_timermod/src/%.h
	$(TOOLPFIX)-gcc $(CFLAGS) -c $< -o $@
