##
# Copyright (C) 2010 Beat Küng <beat-kueng@gmx.net>
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#

ifneq ($(KERNELRELEASE),)
	obj-m := navilock.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
install:
	$(MAKE) INSTALL_MOD_DIR=kernel/drivers/usb/serial -C $(KERNELDIR) M=$(PWD) modules_install
	depmod -a
clean:
	rm *.o *.ko *.mod.c .*.cmd || true
endif

