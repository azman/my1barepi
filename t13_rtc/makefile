# to generate raspberry pi bare-metal code (kernel.img)
TARGET = kernel.img

# multiple source files (list them here as object files!)
OBJLST = boot.o main.o gpio.o timer.o i2c.o utils.o
ifeq ($(USE_OLED),yes)
	CFLAGS += -DUSE_OLED
	OBJLST += font_oled.o oled1306.o
else
	OBJLST += mailbox.o barrier.o font.o video.o
endif

# default rule
all: $(TARGET)

# get project's common makefile stuffs
include ../makefile

# in case we need customization
CFLAGS += -O2
