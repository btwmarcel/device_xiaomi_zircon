#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit_only.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit from zircon device
$(call inherit-product, device/xiaomi/zircon/device.mk)

# Inherit some common Lineage stuff.
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

PRODUCT_DEVICE := zircon
PRODUCT_NAME := lineage_zircon
PRODUCT_BRAND := Redmi
PRODUCT_MODEL := Redmi Note 13 Pro+ 5G
PRODUCT_MANUFACTURER := Xiaomi

PRODUCT_GMS_CLIENTID_BASE := android-xiaomi

PRODUCT_BUILD_PROP_OVERRIDES += \
    BuildDesc="missi-user 16 BP2A.250605.031.A3 OS3.0.302.0.WNOMIXM release-keys" \
    BuildFingerprint=Xiaomi/missi/missi:16/BP2A.250605.031.A3/OS3.0.302.0.WNOMIXM:user/release-keys
