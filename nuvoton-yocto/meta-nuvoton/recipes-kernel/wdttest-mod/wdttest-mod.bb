SUMMARY = "Example of Raspberry Pi driver that simulates triggering watchdog"
DESCRIPTION = "${SUMMARY}"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = "file://Makefile \
           file://wdttest-mod.c \
           file://COPYING \
          "

S = "${WORKDIR}"

RPROVIDES:${PN} += "kernel-module-wdttest-mod"
