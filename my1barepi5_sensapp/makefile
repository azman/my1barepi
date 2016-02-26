# to generate raspberry pi bare-metal code (kernel.img)
TARGET = kernel.img

# multiple source files (list them here as object files!)
OBJLST = boot.o gpio.o timer.o 7seg.o main.o

# default rule
all: $(TARGET)

# get project's common makefile stuffs
include ../makefile

# specify additional module header location
CFLAGS += -I../my1barepiZ_modules/src/

# specify additional module source location
%.o: ../my1barepiZ_modules/src/%.c ../my1barepiZ_modules/src/%.h
	$(TOOLPFIX)-gcc $(CFLAGS) -c $< -o $@
