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
AFLAGS += -I../tZZ_modules/src/
# compiler option(s)
CFLAGS += -I../tZZ_modules/src/
ifeq ($(RASPI),3)
# for raspberry pi 3 (2016)
AFLAGS += -mfpu=crypto-neon-fp-armv8
CFLAGS += -march=armv8-a+crc -mtune=cortex-a53 -mfpu=crypto-neon-fp-armv8
CFLAGS += -DRASPI3
else ifeq ($(RASPI),2)
# for raspberry pi 2 (2015)
AFLAGS += -mfpu=neon-vfpv4
CFLAGS += -march=armv7-a -mtune=cortex-a7 -mfpu=neon-vfpv4
CFLAGS += -DRASPI2
else
AFLAGS += -mfpu=vfp
CFLAGS += -march=armv6zk -mtune=arm1176jzf-s -mfpu=vfp
endif
AFLAGS += -mfloat-abi=hard
CFLAGS += -mfloat-abi=hard -mhard-float
CFLAGS += -nostdlib -nostartfiles -ffreestanding -Wall
# linker option(s)
LFLAGS += --no-undefined

# prevent make from automatically removing these!
TEMPS = $(patsubst src/%.s,%.o,$(wildcard src/*.s))
TEMPS += $(patsubst src/%.s,%.elf,$(wildcard src/*.s))
TEMPS += $(patsubst src/%.c,%.o,$(wildcard src/*.c))
TEMPS += $(patsubst src/%.c,%.elf,$(wildcard src/*.c))
.SECONDARY: $(TEMPS)

THIS_PATH:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
THIS_NAME:=$(shell basename $(THIS_PATH))

info:
	@echo "Go to respective folders to build the tutorial code(s) you need!"

ifeq ($(THIS_NAME),my1barepi)

define rule_template
$(1):
	@echo -n "-- Cleaning $(2) => "
	@make --no-print-directory -C $(2) clean
endef
ALL_LIST=$(wildcard t0*_*) $(wildcard t1*_*) $(wildcard t2*_*)
ALL_RULE=$(foreach tuts, $(ALL_LIST), $(tuts)_rule)
$(foreach tuts, $(ALL_LIST),$(eval $(call rule_template,$(tuts)_rule,$(tuts))))

clean: $(ALL_RULE)

else

clean:
	rm -rf *.img *.elf *.lst *.map *.o

pi: $(TARGET)

new: clean pi

$(TARGET): $(TOPELF)
	$(TOOLPFIX)-objcopy $< -O binary $@

$(TOPELF): $(OBJLST) $(LINKER)
	$(TOOLPFIX)-ld $(LFLAGS) $(OBJLST) -Map $(MAP) -o $@ -T $(LINKER)
	$(TOOLPFIX)-objdump -d $@ > $(LST)

%.o: src/%.s src/%.h
	$(TOOLPFIX)-as $(AFLAGS) $< -o $@

%.o: src/%.s
	$(TOOLPFIX)-as $(AFLAGS) $< -o $@

%.o: ../tZZ_modules/src/%.s ../tZZ_modules/src/%.h
	$(TOOLPFIX)-as $(AFLAGS) $< -o $@

%.o: ../tZZ_modules/src/%.s
	$(TOOLPFIX)-as $(AFLAGS) $< -o $@

%.o: src/%.c src/%.h
	$(TOOLPFIX)-gcc $(CFLAGS) -c $< -o $@

%.o: src/%.c
	$(TOOLPFIX)-gcc $(CFLAGS) -c $< -o $@

%.o: ../tZZ_modules/src/%.c ../tZZ_modules/src/%.h
	$(TOOLPFIX)-gcc $(CFLAGS) -c $< -o $@

endif
