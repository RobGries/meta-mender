FILESEXTRAPATHS_prepend_dragonboard-410c := "${THISDIR}/patches:"

SRC_URI_remove_dragonboard-410c += " file://0003-Integration-of-Mender-boot-code-into-U-Boot.patch"
SRC_URI_append_dragonboard-410c += " file://dragonboard-Integration-of-Mender-boot-code-into-U-Boot.patch"
SRC_URI_append_dragonboard-410c += " file://dragonboard_mender_auto_configured.patch"
SRC_URI_append_dragonboard-410c += " file://0001-Do-not-set-mmc-env-partition-in-DTS.patch"

# Force a specific selection of MENDER_BOOT_KERNEL_TYPE that
# differs from KERNEL_IMAGETYPE.  We have to manually create
# the uImage since the upstream kernel does not seem to support
# it directly so we can't rely on the default value of
# KERNEL_IMAGETYPE.
MENDER_KERNEL_IMAGETYPE_FORCE_dragonboard-410c = "uImage"
