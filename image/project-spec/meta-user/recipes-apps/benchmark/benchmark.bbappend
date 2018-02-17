
do_install_append() {
		 install -d ${D}/home/root
     install -m 0644 ${S}/tulips.jpg ${D}/home/root/tulips.jpg
}

FILES_${PN} += " \
            /home/root/* \
         "
