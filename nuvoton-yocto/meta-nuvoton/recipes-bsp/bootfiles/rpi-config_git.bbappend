do_deploy:append() {
        echo "# Enable my irqtestnode" >> ${DEPLOYDIR}/bootfiles/config.txt
        echo "dtoverlay=irqtestnode" >> ${DEPLOYDIR}/bootfiles/config.txt
}
