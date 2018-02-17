#
# This file is the fpga-matcher recipe.
#

SUMMARY = "FPGA Matcher"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://main.cpp \
           file://CMakeLists.txt \
           file://include/* \
           file://src/* \
		  "

S = "${WORKDIR}"
CFLAGS_prepend = "-I ${S}/include -fPIC "


inherit cmake
