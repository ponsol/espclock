ifneq (${KERNELRELEASE},)
        obj-m  = espclock.o
        espclock-y := espclock_main.o espclock_sup.o espclock_cdev.o
else
        KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
        KERNEL_DIR = /home/pi/linux
        MODULE_DIR := $(shell pwd)

.PHONY: all

all: modules

.PHONY:modules

modules:
	${MAKE} -C ${KERNEL_DIR} M=${MODULE_DIR} 

clean:
	rm -f *.o *.ko *.mod.c .*.o .*.ko .*.mod.c .*.cmd *~
	rm -f Module.symvers Module.markers modules.order
	rm -rf .tmp_versions
endif
