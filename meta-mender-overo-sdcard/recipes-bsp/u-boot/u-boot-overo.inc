FILESEXTRAPATHS_prepend_overo := "${THISDIR}/patches:"

# Overo board is using 2016.11 u-boot, need to apply a patch that matches our version
SRC_URI_remove_overo += " file://0003-Integration-of-Mender-boot-code-into-U-Boot.patch"
SRC_URI_append_overo += " file://0003-U-Boot-pre-2017.03-integration-of-Mender-boot-code-i.patch"

# This will likely not work at the moment.  The default assumes the UBoot environment
# is in NAND.  For proper SDCard based Mender updates, that should probably be moved
# to MMC so we can easily update it.
# UBI is currently tested and working; this is being kept for archival purposes.
SRC_URI_append_overo += " file://0001-u-boot-overo-Mender-autoconfiguration.patch"

# UBoot environment on Overo boards is 128kB.
# 2 copies are maintained in the environ MTD partition
BOOTENV_SIZE_overo = "0x20000"
BOOTENV_RANGE_overo = "0x20000"
