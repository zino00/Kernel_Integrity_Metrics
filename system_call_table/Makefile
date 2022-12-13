obj-m += change_scb.o netlink.o

KDIR =/home/zino/kernel/linux-5.4.98/

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	
clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order *.cmd *.d *.mod .*.cmd