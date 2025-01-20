# Makefile for the Deepak_2002 Linux Driver

obj-m += file_operation.o

# KERNELDIR = /path/to/your/kernel
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
