SUMMARY = "Nuvoton custom image with drvs and test apps"

IMAGE_FEATURES += "splash"

LICENSE = "MIT"

inherit core-image

IMAGE_INSTALL += " python3 watchdog"
IMAGE_INSTALL += " custom-file"
IMAGE_INSTALL += " nuvoton-packagegroup-drvs nuvoton-packagegroup-apps"
