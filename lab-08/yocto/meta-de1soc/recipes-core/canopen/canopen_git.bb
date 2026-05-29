# Recipe created by recipetool
# This is the basis of a recipe and may need further editing in order to be fully functional.
# (Feel free to remove these comments when editing.)

# WARNING: the following LICENSE and LIC_FILES_CHKSUM values are best guesses - it is
# your responsibility to verify that the values are complete and correct.
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://CANopenNode/LICENSE;md5=3b83ef96387f14655fc854ddc3c6bd57 \
                    file://LICENSE;md5=3b83ef96387f14655fc854ddc3c6bd57"

SRC_URI = "gitsm://github.com/CANopenNode/CANopenLinux;protocol=https;branch=master"

# Modify these as desired
PV = "1.0+git${SRCPV}"
SRCREV = "f1348d4072cdabea4c3435a13c721ac29ab4cc91"

S = "${WORKDIR}/git"
TARGET_CC_ARCH += "${LDFLAGS}"

# NOTE: this is a Makefile-only piece of software, so we cannot generate much of the
# recipe automatically - you will need to examine the Makefile yourself and ensure
# that the appropriate arguments are passed in.

do_compile () {
	# You will almost certainly need to add additional arguments here
	oe_runmake
}

do_install () {
	# NOTE: unable to determine what to put here - there is a Makefile but no
	# target named "install", so you will need to define this yourself
	install -D -m 0755 ${B}/canopend ${D}/usr/bin/canopend
}

