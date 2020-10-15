# to generate raspberry pi bare-metal code (kernel.img)
TARGET = kernel.img

# multiple source files (list them here as object files!)
OBJLST = boot.o main.o gpio.o timer.o uart.o bluez.o string.o utils.o
OBJLST += mailbox.o barrier.o video.o font.o

# default rule
all: $(TARGET)

# get project's common makefile stuffs
include ../makefile

# in case we need customization
CFLAGS += -O2
