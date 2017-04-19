# common makefile to generate raspberry pi bare-metal code (kernel.img)

ifeq ($(OS),Windows_NT)
TOOLPATH ?= /c/users/public/tool/xtool-arm/bin/
else
TOOLPATH ?= /home/share/tool/xtool-arm/bin/
endif
TOOLPFIX ?= $(TOOLPATH)arm-none-eabi

# multiple source files (list them here as object files!)
OBJLST ?= boot.o main.o

LINKER ?= ../linker.ld
TARGET ?= kernel.img
TOPELF ?= main.elf
LST ?= kernel.lst
MAP ?= kernel.map

# assembler option(s)
AFLAGS +=
# compiler option(s)
ifeq ($(RASPI),3)
# for raspberry pi 3 (2016)
CFLAGS += -march=armv8-a+crc -mtune=cortex-a53 -mfpu=crypto-neon-fp-armv8
CFLAGS += -DRASPI3
else ifeq ($(RASPI),2)
# for raspberry pi 2 (2015)
CFLAGS += -march=armv7-a -mtune=cortex-a7 -mfpu=neon-vfpv4
CFLAGS += -DRASPI2
else
CFLAGS += -march=armv6zk -mtune=arm1176jzf-s -mfpu=vfp
endif
CFLAGS += -mfloat-abi=hard
CFLAGS += -nostdlib -nostartfiles -ffreestanding -Wall
# linker option(s)
LFLAGS += --no-undefined

# prevent make from automatically removing these!
TEMPS = $(patsubst src/%.s,%.o,$(wildcard src/*.s))
TEMPS += $(patsubst src/%.s,%.elf,$(wildcard src/*.s))
TEMPS += $(patsubst src/%.c,%.o,$(wildcard src/*.c))
TEMPS += $(patsubst src/%.c,%.elf,$(wildcard src/*.c))
.SECONDARY: $(TEMPS)

info:
	@echo "Go to respective folders to build the tutorial code(s) you need!"

pi: $(TARGET)

clean:
	rm -rf *.img *.elf *.lst *.map *.o

new: clean pi

$(TARGET): $(TOPELF)
	$(TOOLPFIX)-objcopy $< -O binary $@

$(TOPELF): $(OBJLST) $(LINKER)
	$(TOOLPFIX)-ld $(LFLAGS) $^ -Map $(MAP) -o $@ -T $(LINKER)
	$(TOOLPFIX)-objdump -d $@ > $(LST)

%.o: src/%.s
	$(TOOLPFIX)-as $(AFLAGS) $< -o $@

%.o: src/%.c src/%.h
	$(TOOLPFIX)-gcc $(CFLAGS) -c $< -o $@

%.o: src/%.c
	$(TOOLPFIX)-gcc $(CFLAGS) -c $< -o $@
