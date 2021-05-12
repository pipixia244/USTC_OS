#目录
KERNEL_DIR = /lib/modules/`unmae -r`/build

#解决 loading out-of-tree module taints kernel.
#和 module verification failed: signature and/or required key missing - tainting kernel
#的污染警告(虽然不影响结果)
CONFIG_MODULE_SIG=n
CONFIG_MODULE_SIG_FORCE=n
CONFIG_MODULE_SIG_ALL=n

obj-m += helloworld.o

#将空格更改为tab
all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(shell pwd) modules

clean:
	rm -rf *.o  *.mod.c .*.cmd .tmp_versions *.order *.symvers