FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

unset KBUILD_DEFCONFIG

SRC_URI += "file://wdttestnode-overlay.dts;subdir=git/arch/${ARCH}/boot/dts/overlays \
	    file://enable-mfd-nct6694.cfg \
	    file://enable-gpio-nct6694.cfg \
	    file://enable-i2c-nct6694.cfg \
	    file://enable-can-nct6694.cfg \
	    file://enable-wdt-nct6694.cfg \
	    file://enable-iio-nct6694.cfg \
	    file://enable-hwmon-nct6694.cfg \
	    file://enable-pwm-nct6694.cfg \
	    file://enable-rtc-nct6694.cfg \
"


KERNEL_DEVICETREE:append = " overlays/wdttestnode.dtbo "
