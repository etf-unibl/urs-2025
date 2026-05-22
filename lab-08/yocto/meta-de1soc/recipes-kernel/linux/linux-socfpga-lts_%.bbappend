FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://de1_soc_defconfig \
            file://socfpga_cyclone5_de1_soc.dts"

do_kernel_metadata:prepend(){
    cp ${WORKDIR}/de1_soc_defconfig ${S}/arch/arm/configs
    cp ${WORKDIR}/socfpga_cyclone5_de1_soc.dts ${S}/arch/arm/boot/dts
    if ! grep -q "socfpga_cyclone5_de1_soc.dtb" ${S}/arch/arm/boot/dts/Makefile; then
        echo "dtb-\$(CONFIG_ARCH_INTEL_SOCFPGA) += socfpga_cyclone5_de1_soc.dtb" >> ${S}/arch/arm/boot/dts/Makefile
    fi
}

KBUILD_DEFCONFIG = "de1_soc_defconfig"
