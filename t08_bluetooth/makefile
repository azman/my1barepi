# to generate raspberry pi bare-metal code (kernel.img)
TARGET = kernel.img

# multiple source files (list them here as object files!)
OBJLST = boot.o main.o gpio.o timer.o uart.o uartbb.o bluez.o string.o utils.o
OBJLST += mailbox.o barrier.o

# default rule
all: $(TARGET)

# get project's common makefile stuffs
include ../makefile

# in case we need customization
#CFLAGS += -O2

# specify additional module header location
CFLAGS += -I../tZZ_modules/src/

# specify additional module source location
%.o: ../tZZ_modules/src/%.c ../tZZ_modules/src/%.h
	$(TOOLPFIX)-gcc $(CFLAGS) -c $< -o $@

# specify additional module header location
AFLAGS += -I../tZZ_modules/src/

# specify additional module source location
%.o: ../tZZ_modules/src/%.s ../tZZ_modules/src/%.h
	$(TOOLPFIX)-as $(AFLAGS) $< -o $@
