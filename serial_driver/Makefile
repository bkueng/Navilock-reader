ifneq ($(KERNELRELEASE),)
	obj-m := navilock.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	rm *.o *.ko *.mod.c .*.cmd || true
endif

