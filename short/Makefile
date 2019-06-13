obj-m:=short.o
KERNELDIR:=/lib/modules/4.18.0-20-generic/build
# KERNELDIR:=/lib/modules/2.6.23/build
PWD:=$(shell pwd)
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules std=c11
modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install std=c11