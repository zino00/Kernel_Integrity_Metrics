obj-m += kernel.o

KDIR =/home/zino/kernel/linux-2.6.18/

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	
clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order *.cmd *.d