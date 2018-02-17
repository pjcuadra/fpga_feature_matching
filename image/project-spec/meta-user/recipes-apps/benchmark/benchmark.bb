#
# This is the Feature Matching Demo apllication recipe
#
#

SUMMARY = "Feature Matching application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
SRC_URI = "http://en.bcdn.biz/Images/2016/7/22/9c0e0ba0-4595-4c6a-b80a-5b7f2a726d8c.jpg;downloadfilename=tulips.jpg \
	         file://main.cpp \
           file://CMakeLists.txt \
        "
SRC_URI[md5sum] = "341aa146bcbf78d5fcd14f936857b9d8"

DEPENDS = "opencv"

S = "${WORKDIR}"
CFLAGS_prepend = "-I ${S}/include"


inherit cmake
