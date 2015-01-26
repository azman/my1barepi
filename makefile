# makefile to manage my1barepi

TUT_LIST = my1barepi0_intro2a my1barepi0_intro2c my1barepi1_gpiomod
TUT_LIST += my1barepi2_timermod my1barepi3_play7seg my1barepi4_uartmod
TUT_LIST += my1barepi5_sensapp my1barepiX_loader

all:
	@for a in $(TUT_LIST); do make -C $${a}; done

clean:
	@for a in $(TUT_LIST); do make -C $${a} $@; done
