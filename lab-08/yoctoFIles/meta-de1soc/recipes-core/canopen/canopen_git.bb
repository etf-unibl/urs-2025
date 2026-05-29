LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://CANopenNode/LICENSE;md5=3b83ef96387f14655fc854ddc3c6bd57 \
                    file://LICENSE;md5=3b83ef96387f14655fc854ddc3c6bd57"

inherit pkgconfig

SRC_URI = "gitsm://github.com/CANopenNode/CANopenLinux;protocol=https;branch=master"

PV = "1.0+git${SRCPV}"
SRCREV = "f1348d4072cdabea4c3435a13c721ac29ab4cc91"

S = "${WORKDIR}/git"

TARGET_CC_ARCH += "${LDFLAGS}"

do_compile () {
    oe_runmake
}

do_install () {
    install -D -m 0755 ${B}/canopend ${D}/usr/bin/canopend
}

PARALLEL_MAKE = ""
B = "${S}"
