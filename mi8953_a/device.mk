#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

TARGET_DEVICE_PATH := device/xiaomi/mi89xx-mainline/mi8953_a

# Inherit from parent
$(call inherit-product, device/xiaomi/mi89xx-mainline/device.mk)

# AAPT
PRODUCT_AAPT_PREF_CONFIG := xxhdpi

# Audio
TARGET_AUDIO_MAINLINE_UCM_PROFILES := \
    xiaomi-daisy \
    xiaomi-markw \
    xiaomi-mido \
    xiaomi-onclite \
    xiaomi-rosy \
    xiaomi-tissot \
    xiaomi-vince \
    xiaomi-ysl

# Boot animation
TARGET_SCREEN_HEIGHT := 1920
TARGET_SCREEN_WIDTH := 1080

# ConsumerIR
TARGET_HAS_IR := true

# Dalvik heap
$(call inherit-product, frameworks/native/build/phone-xhdpi-2048-dalvik-heap.mk)

# Fingerprint
PRODUCT_PACKAGES += \
    android.hardware.biometrics.fingerprint@2.1-service.xiaomi_vince

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.fingerprint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.fingerprint.xml

# Firmware
PRODUCT_COPY_FILES += \
    vendor/xiaomi/msm8953-common/proprietary/vendor/firmware/a506_zap.b00:$(TARGET_COPY_OUT_ODM)/firmware/qcom/msm8953/xiaomi/a506_zap.b00 \
    vendor/xiaomi/msm8953-common/proprietary/vendor/firmware/a506_zap.b01:$(TARGET_COPY_OUT_ODM)/firmware/qcom/msm8953/xiaomi/a506_zap.b01 \
    vendor/xiaomi/msm8953-common/proprietary/vendor/firmware/a506_zap.b02:$(TARGET_COPY_OUT_ODM)/firmware/qcom/msm8953/xiaomi/a506_zap.b02 \
    vendor/xiaomi/msm8953-common/proprietary/vendor/firmware/a506_zap.mdt:$(TARGET_COPY_OUT_ODM)/firmware/qcom/msm8953/xiaomi/a506_zap.mdt \
    vendor/xiaomi/onclite/proprietary/vendor/firmware/a506_zap.b00:$(TARGET_COPY_OUT_ODM)/firmware/qcom/msm8953/xiaomi/onclite/a506_zap.b00 \
    vendor/xiaomi/onclite/proprietary/vendor/firmware/a506_zap.b01:$(TARGET_COPY_OUT_ODM)/firmware/qcom/msm8953/xiaomi/onclite/a506_zap.b01 \
    vendor/xiaomi/onclite/proprietary/vendor/firmware/a506_zap.b02:$(TARGET_COPY_OUT_ODM)/firmware/qcom/msm8953/xiaomi/onclite/a506_zap.b02 \
    vendor/xiaomi/onclite/proprietary/vendor/firmware/a506_zap.mdt:$(TARGET_COPY_OUT_ODM)/firmware/qcom/msm8953/xiaomi/onclite/a506_zap.mdt \
    vendor/xiaomi/rosy/proprietary/vendor/firmware/a506_zap.b00:$(TARGET_COPY_OUT_ODM)/firmware/qcom/msm8953/xiaomi/rosy/a506_zap.b00 \
    vendor/xiaomi/rosy/proprietary/vendor/firmware/a506_zap.b01:$(TARGET_COPY_OUT_ODM)/firmware/qcom/msm8953/xiaomi/rosy/a506_zap.b01 \
    vendor/xiaomi/rosy/proprietary/vendor/firmware/a506_zap.b02:$(TARGET_COPY_OUT_ODM)/firmware/qcom/msm8953/xiaomi/rosy/a506_zap.b02 \
    vendor/xiaomi/rosy/proprietary/vendor/firmware/a506_zap.mdt:$(TARGET_COPY_OUT_ODM)/firmware/qcom/msm8953/xiaomi/rosy/a506_zap.mdt

PRODUCT_PACKAGES += \
    all_symlink_firmware_mi8953_a

_src := vendor/xiaomi/oxygen/proprietary/vendor/firmware/wlan/prima/WCNSS_qcom_wlan_nv.bin
_dst := $(TARGET_COPY_OUT_ODM)/firmware/qcom/msm8953/xiaomi/oxygen/WCNSS_qcom_wlan_nv.bin
$(eval $(if $(wildcard $(_src)),PRODUCT_COPY_FILES += $(_src):$(_dst),$(warning $(_src) not found)))

# Init
PRODUCT_PACKAGES += \
    fstab.mi8953_a \
    fstab.mi8953_a.ramdisk \
    init.mi8953_a.rc \
    ueventd.mi8953_a.rc

ifneq ($(MI8953_USE_ANDROID_COMMON_KERNEL),true)
PRODUCT_PACKAGES += \
    use_memfd.rc
endif

$(call soong_config_set,mainline_common_libinit,set_properties_from,devicetree)

# Input
PRODUCT_PACKAGES += \
    gf3208.kl

# Kernel
ifneq ($(MI8953_USE_ANDROID_COMMON_KERNEL),true)
PRODUCT_PACKAGES += \
    modules.load.normal
endif

# Overlay
DEVICE_PACKAGE_OVERLAYS += \
    $(TARGET_DEVICE_PATH)/overlays/overlay

# Properties
PRODUCT_VENDOR_PROPERTIES += \
    vendor.remoteproc.4080000_remoteproc.ignore=1

# SoC
TARGET_QCOM_SOC_FAMILY := msm8953

# Soong namespaces
PRODUCT_SOONG_NAMESPACES += \
    $(TARGET_DEVICE_PATH) \
    kernel/mainline/configs

# Inherit the proprietary files
$(call inherit-product-if-exists, vendor/xiaomi/mi8953_a/mi8953_a-vendor.mk)
