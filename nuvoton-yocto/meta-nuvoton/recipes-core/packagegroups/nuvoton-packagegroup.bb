DESCRIPTION = "Nuvoton drivers & test applications packagegroup"
SUMMARY = "Nuvoton packagegroup - drvs/testapps"

inherit packagegroup

LICENSE = "MIT"
                                             
PACKAGES = "\
    ${PN}-drvs \
    ${PN}-apps \
"

RDEPENDS:${PN}-drvs = "\
    nct6694-usbdrvs \
    wdttest-mod \
"
                                                
RDEPENDS:${PN}-apps = "\
    i2c-tools \
    can-utils \
    libgpiod-tools \
    dfu-util \
"
