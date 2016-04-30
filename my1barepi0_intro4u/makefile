# to generate raspberry pi bare-metal code (kernel.img)
TARGET = kernel.img

# multiple source files (list them here as object files!)
OBJLST = boot.o main.o

# default rule
all: $(TARGET)

# in case we need customization
CFLAGS += -O2
# -02 optimization got us (160-140) bytes savings

# get project's common makefile stuffs
include ../makefile
