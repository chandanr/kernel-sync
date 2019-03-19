
# force no-pie for distro compilers that enable pie by default
CFLAGS_EXTRA += $(CFLAGS_EXTRA) -fno-pie

obj-m += nosync-module.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
