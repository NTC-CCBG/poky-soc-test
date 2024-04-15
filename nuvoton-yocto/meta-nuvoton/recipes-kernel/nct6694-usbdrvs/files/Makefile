obj-m := nct6694-usb_mfd.o
obj-m += nct6694-i2c.o
obj-m += nct6694-gpio.o
obj-m += nct6694-canfd.o
obj-m += nct6694-wdt.o

SRC := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC)
	echo "$(KERNEL_SRC)" > test.txt
	echo "$(WORKDIR)" >> test.txt
	echo "$(SRC)" >> test.txt

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules_install

clean:
	rm -f *.o *~ core .depend .*.cmd *.ko *.mod.c
	rm -f Module.markers Module.symvers modules.order
	rm -rf .tmp_versions Modules.symvers
