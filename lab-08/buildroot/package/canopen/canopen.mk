CANOPEN_VERSION = v4.0
CANOPEN_SITE = https://github.com/CANopenNode/CANopenLinux
CANOPEN_LICENSE = Apache-2.0
CANOPEN_LICENSE_FILES = LICENSE
CANOPEN_SITE_METHOD=git
CANOPEN_GIT_SUBMODULES = YES

define CANOPEN_BUILD_CMDS
    $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D) all
endef

define CANOPEN_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/canopend $(TARGET_DIR)/usr/bin/canopend
endef

$(eval $(generic-package))
