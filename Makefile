MODULE_NAME := netlink_app
obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-objs := main.o

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

PHONY: all clean