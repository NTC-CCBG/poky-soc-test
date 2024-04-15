DESCRIPTION = "Install specific files into the root filesystem"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI += " \
    file://Kernel_NCT6694D_eSIO_Signed.bin \
"

S = "${WORKDIR}"

do_install() {
    install -d 0644 ${D}/home/root/Testdir
    install -m 0755 ${WORKDIR}/Kernel_NCT6694D_eSIO_Signed.bin ${D}/home/root/Testdir
}

FILES:${PN} += " \
    /home/root/Testdir/Kernel_NCT6694D_eSIO_Signed.bin \
"
