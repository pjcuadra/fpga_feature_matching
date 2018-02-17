
DEPEDNS_append = " fpga-matcher"

do_configure_prepend () {
	#ln -sf 
	echo "Test top dir - ${TOPDIR}/../../opencv_module" >> /tmp/bbapp.test
	pwd >> /tmp/bbapp.test
	echo "Test S ${S}/../contrib/modules/opencv_module" >> /tmp/bbapp.test
	ln -sf ${TOPDIR}/../../opencv_module ${S}/../contrib/modules/fpga_matcher
	
}
