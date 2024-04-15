SUMMARY = "Nuvoton USB interface drivers for NCT6694"
DESCRIPTION = "${SUMMARY}"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = "file://Makefile \
           file://nct6694-usb_mfd.c \
	   file://nct6694-usb_mfd.h \
	   file://nct6694-i2c.c \
	   file://nct6694-gpio.c \
	   file://nct6694-canfd.c \
	   file://nct6694-wdt.c \
           file://COPYING \
          "

S = "${WORKDIR}"

RPROVIDES:${PN} += "kernel-module-usbdrvs"
