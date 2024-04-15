FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

unset KBUILD_DEFCONFIG

SRC_URI += "file://defconfig \
            file://irqtestnode-overlay.dts;subdir=git/arch/${ARCH}/boot/dts/overlays \
"

KERNEL_DEVICETREE:append = " overlays/irqtestnode.dtbo"
