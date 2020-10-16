# to generate raspberry pi bare-metal code (kernel.img)
TARGET = kernel.img

# multiple source files (list them here as object files!)
OBJLST = boot.o gpio.o timer.o uart.o string.o main.o
OBJLST += utils.o fpu.o
OBJLST += mailbox.o barrier.o font.o video.o

# default rule
all: $(TARGET)

# get project's common makefile stuffs
include ../makefile

# in case we need customization
CFLAGS += -O2
