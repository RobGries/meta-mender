inherit mender-helpers

DEPENDS += "coreutils-native"

BOOTENV_SIZE ??= "0x20000"
BOOTENV_SIZE[doc] = "Size of bootloader environment"

BOOTENV_RANGE ??= ""
BOOTENV_RANGE[doc] = "Range occupied by bootloader environment"

DEPLOYDIR = "${WORKDIR}/deploy-${PN}"

################################################################################
# Generic patches.
################################################################################
FILESEXTRAPATHS_prepend := "${THISDIR}/patches:"

SRC_URI_append = " file://0001-Add-missing-header-which-fails-on-recent-GCC.patch"
SRC_URI_append = " file://0002-Generic-boot-code-for-Mender.patch"
SRC_URI_append = " file://0003-Integration-of-Mender-boot-code-into-U-Boot.patch"
SRC_URI_append = " file://0004-Disable-CONFIG_BOOTCOMMAND-and-enable-CONFIG_MENDER_.patch"

################################################################################
# Generic tasks.
################################################################################
do_provide_mender_defines() {
    if ! ${@bb.utils.contains('DISTRO_FEATURES', 'mender-uboot', 'true', 'false', d)}; then
        bbfatal "To compile U-Boot with mender you have to add 'mender-uboot' to MENDER_FEATURES_ENABLE or DISTRO_FEATURES."
    fi

    if [ $(expr ${MENDER_STORAGE_RESERVED_RAW_SPACE} % \( ${MENDER_PARTITION_ALIGNMENT_KB} \* 1024 \* 2 \)) -ne 0 ]; then
        bbfatal "MENDER_STORAGE_RESERVED_RAW_SPACE is not an even multiple of MENDER_PARTITION_ALIGNMENT_KB."
    fi

    if [ ${MENDER_BOOTENV_TOTAL_ALIGNED_SIZE} -gt ${MENDER_STORAGE_RESERVED_RAW_SPACE} ]; then
        bbfatal "BOOTENV_SIZE (${BOOTENV_SIZE}) is too big to fit two copies inside MENDER_STORAGE_RESERVED_RAW_SPACE (${MENDER_STORAGE_RESERVED_RAW_SPACE}) with proper alignment. Please increase MENDER_STORAGE_RESERVED_RAW_SPACE manually and make sure it is an *even* multiple of MENDER_PARTITION_ALIGNMENT_KB (in bytes)."
    fi

    if [ -n "${MENDER_BOOT_PART}" ]; then
        MENDER_BOOT_PART_NUMBER=`get_part_number_from_device ${MENDER_BOOT_PART}`
        MENDER_BOOT_PART_NUMBER_HEX=`get_part_number_hex_from_device ${MENDER_BOOT_PART}`
    fi

    MENDER_ROOTFS_PART_A_NUMBER=`get_part_number_from_device ${MENDER_ROOTFS_PART_A}`
    MENDER_ROOTFS_PART_A_NUMBER_HEX=`get_part_number_hex_from_device ${MENDER_ROOTFS_PART_A}`
    MENDER_ROOTFS_PART_B_NUMBER=`get_part_number_from_device ${MENDER_ROOTFS_PART_B}`
    MENDER_ROOTFS_PART_B_NUMBER_HEX=`get_part_number_hex_from_device ${MENDER_ROOTFS_PART_B}`

    if [ -n "${MENDER_UBOOT_STORAGE_INTERFACE}" ]; then
        MENDER_UBOOT_STORAGE_INTERFACE=${MENDER_UBOOT_STORAGE_INTERFACE}
    else
        MENDER_UBOOT_STORAGE_INTERFACE=`get_uboot_interface_from_device ${MENDER_STORAGE_DEVICE}`
    fi
    if [ -n "${MENDER_UBOOT_STORAGE_DEVICE}" ]; then
        MENDER_UBOOT_STORAGE_DEVICE=${MENDER_UBOOT_STORAGE_DEVICE}
    else
        MENDER_UBOOT_STORAGE_DEVICE=`get_uboot_device_from_device ${MENDER_STORAGE_DEVICE}`
    fi

    if [ -n "${MENDER_KERNEL_IMAGETYPE_FORCE}" ]; then
        MENDER_KERNEL_IMAGETYPE="${MENDER_KERNEL_IMAGETYPE_FORCE}"
    elif [ -n "${KERNEL_IMAGETYPE}" ]; then
        MENDER_KERNEL_IMAGETYPE="${KERNEL_IMAGETYPE}"
    else
        MENDER_KERNEL_IMAGETYPE="zImage"
    fi

    case "$MENDER_KERNEL_IMAGETYPE" in
        *uImage*)
            MENDER_BOOT_KERNEL_TYPE=bootm
            ;;
        *zImage*)
            MENDER_BOOT_KERNEL_TYPE=bootz
            ;;
        Image*)
            MENDER_BOOT_KERNEL_TYPE=booti
            ;;
        *)
            MENDER_BOOT_KERNEL_TYPE=bootz
            ;;
    esac

    # Strip leading and trailing whitespace, then newline divide, and remove dtbo's.
    MENDER_DTB_NAME="$(echo "${KERNEL_DEVICETREE}" | sed -r 's/(^\s*)|(\s*$)//g; s/\s+/\n/g' | sed -ne '/\.dtbo$/b; p')"

    if [ -z "$MENDER_DTB_NAME" ]; then
        bbfatal "Did not find a dtb specified in KERNEL_DEVICETREE"
        exit 1
    fi

    DTB_COUNT=$(echo "$MENDER_DTB_NAME" | wc -l)

    if [ "$DTB_COUNT" -ne 1 ]; then
        bbwarn "Found more than one dtb specified in KERNEL_DEVICETREE. Only one should be specified. Choosing the last one."
        MENDER_DTB_NAME="$(echo "$MENDER_DTB_NAME" | tail -1)"
    fi

    # Now strip any subdirectories off.  Some kernel builds require KERNEL_DEVICETREE to be defined, for example,
    # as qcom/apq8016-sbc.dtb yet when installed, they go directly in /boot
    MENDER_DTB_NAME="$(basename "$MENDER_DTB_NAME")"

    cat > ${S}/include/config_mender_defines.h <<EOF
/* AUTOGENERATED FILE - DO NOT EDIT! */
/* This file is provided by the meta-mender layer. */

/* Shell variables */
#define MENDER_BOOT_PART_NUMBER $MENDER_BOOT_PART_NUMBER
#define MENDER_BOOT_PART_NUMBER_HEX $MENDER_BOOT_PART_NUMBER_HEX
#define MENDER_ROOTFS_PART_A_NUMBER $MENDER_ROOTFS_PART_A_NUMBER
#define MENDER_ROOTFS_PART_A_NUMBER_HEX $MENDER_ROOTFS_PART_A_NUMBER_HEX
#define MENDER_ROOTFS_PART_B_NUMBER $MENDER_ROOTFS_PART_B_NUMBER
#define MENDER_ROOTFS_PART_B_NUMBER_HEX $MENDER_ROOTFS_PART_B_NUMBER_HEX
#define MENDER_UBOOT_STORAGE_INTERFACE "$MENDER_UBOOT_STORAGE_INTERFACE"
#define MENDER_UBOOT_STORAGE_DEVICE $MENDER_UBOOT_STORAGE_DEVICE

/* BB variables. */
#define MENDER_STORAGE_DEVICE_BASE "${MENDER_STORAGE_DEVICE_BASE}"
#define MENDER_UBOOT_ENV_STORAGE_DEVICE_OFFSET_1 ${MENDER_UBOOT_ENV_STORAGE_DEVICE_OFFSET_1}
#define MENDER_UBOOT_ENV_STORAGE_DEVICE_OFFSET_2 ${MENDER_UBOOT_ENV_STORAGE_DEVICE_OFFSET_2}
#define MENDER_ROOTFS_PART_A_NAME "${MENDER_ROOTFS_PART_A_NAME}"
#define MENDER_ROOTFS_PART_B_NAME "${MENDER_ROOTFS_PART_B_NAME}"
#define MENDER_MTD_UBI_DEVICE_NAME "${MENDER_MTD_UBI_DEVICE_NAME}"

/* For sanity checks. */
#define MENDER_BOOTENV_SIZE ${BOOTENV_SIZE}
#undef MENDER_BOOTENV_RANGE

#define MENDER_BOOT_KERNEL_TYPE "$MENDER_BOOT_KERNEL_TYPE"
#define MENDER_KERNEL_NAME "$MENDER_KERNEL_IMAGETYPE"
#define MENDER_DTB_NAME "$MENDER_DTB_NAME"
EOF

    if [ -n "${BOOTENV_RANGE}" ]; then
        cat >> ${S}/include/config_mender_defines.h <<EOF
#define MENDER_BOOTENV_RANGE ${BOOTENV_RANGE}
EOF
    fi
}
addtask do_provide_mender_defines after do_patch before do_configure

################################################################################
# Helpers and internal variables.
################################################################################

# This should evaluate to the same as MENDER_STORAGE_RESERVED_RAW_SPACE.
# The only reason it's not evaluated the same way is that we don't have the
# necessary information (BOOTENV_SIZE and BOOTENV_RANGE) when evaluating
# MENDER_STORAGE_RESERVED_RAW_SPACE.
MENDER_BOOTENV_TOTAL_ALIGNED_SIZE = "${@mender_get_env_total_aligned_size(${BOOTENV_SIZE}, d.getVar('BOOTENV_RANGE', True) or None, ${MENDER_PARTITION_ALIGNMENT_KB})}"

def mender_get_env_offset(start_offset, index, total_aligned_size):
    if index == 1:
        return "0x%x" % int(start_offset)
    elif index == 2:
        return "0x%x" % int(start_offset + total_aligned_size / 2)
    else:
        raise Exception("env index out of range in mender_get_env_offset: Should not happen")

MENDER_UBOOT_ENV_STORAGE_DEVICE_OFFSET_1 ?= "${@mender_get_env_offset(${MENDER_UBOOT_ENV_STORAGE_DEVICE_OFFSET}, 1, \
                                                                      ${MENDER_BOOTENV_TOTAL_ALIGNED_SIZE})}"
MENDER_UBOOT_ENV_STORAGE_DEVICE_OFFSET_2 ?= "${@mender_get_env_offset(${MENDER_UBOOT_ENV_STORAGE_DEVICE_OFFSET}, 2, \
                                                                      ${MENDER_BOOTENV_TOTAL_ALIGNED_SIZE})}"
