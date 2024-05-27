do_deploy:append() {
        echo "# Enable my wdttestnode" >> ${DEPLOYDIR}/bootfiles/config.txt
        echo "dtoverlay=wdttestnode" >> ${DEPLOYDIR}/bootfiles/config.txt
}
