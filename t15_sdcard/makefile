# to generate raspberry pi bare-metal code (kernel.img)
TARGET = kernel.img

# multiple source files (list them here as object files!)
OBJLST = boot.o main.o gpio.o timer.o sdext.o
OBJLST += mailbox.o barrier.o utils.o font.o video.o spi.o

# default rule
all: $(TARGET)

# get project's common makefile stuffs
include ../makefile

# in case we need customization
CFLAGS += -O2
