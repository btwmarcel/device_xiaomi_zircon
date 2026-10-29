#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.fixups_lib import (
    lib_fixups,
)

from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)

from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)

namespace_imports = [
    'device/xiaomi/zircon',
    'hardware/mediatek',
    'hardware/xiaomi'
]

blob_fixups: blob_fixups_user_type = {
    'vendor/lib64/hw/hwcomposer.mtk_common.so': blob_fixup()
        .binary_regex_replace(
            b'/sys/class/mi_display/disp-DSI-0/brightness_clone\x00',
            b'/sys/class/leds/lcd-backlight/brightness'.ljust(50, b'\x00'),
        ),
    ('vendor/bin/hw/android.hardware.media.c2@1.2-mediatek-64b',): blob_fixup()
        .add_needed('libstagefright_foundation-v33.so'),

    ('vendor/lib64/libcodec2_mtk_vdec.so', 'vendor/lib64/libcodec2_mtk_venc.so'): blob_fixup()
        .replace_needed('libformatter.so', 'libformatter-mtk.so'),

    ('vendor/lib64/hw/sensors.mediatek.V2.0.so', 'vendor/lib64/libcodec2_mtk_c2store.so', 'vendor/lib64/libcodec2_mtk_vdec.so',
        'vendor/lib64/libcodec2_mtk_venc.so', 'vendor/lib64/libcodec2_vpp_AIMEMC_plugin.so', 'vendor/lib64/libcodec2_vpp_AISR_plugin.so',
        'vendor/lib64/libcodec2_vpp_dolby_plugin.so', 'vendor/lib64/libcodec2_vpp_gc_plugin.so', 'vendor/lib64/libcodec2_vpp_mi_plugin.so',
        'vendor/lib64/libcodec2_vpp_qt_plugin.so', 'vendor/lib64/libcodec2_vpp_rs_plugin.so'): blob_fixup()
        .replace_needed('libstagefright_foundation.so', 'libstagefright_foundation-v33.so'),

    'odm/lib64/libmt_mitee@1.3.so': blob_fixup()
        .replace_needed('android.hardware.security.keymint-V2-ndk.so',
                        'android.hardware.security.keymint-V4-ndk.so')
        .replace_needed('libkeymint_support.so',
                    'libkeymint_support_V4.so'),

    (
        'vendor/lib/vendor.mediatek.hardware.pq_aidl-V1-ndk.so',
        'vendor/lib64/vendor.mediatek.hardware.pq_aidl-V1-ndk.so',
        'vendor/lib/vendor.mediatek.hardware.pq_aidl-V2-ndk.so',
        'vendor/lib64/vendor.mediatek.hardware.pq_aidl-V2-ndk.so',
        'vendor/lib/vendor.mediatek.hardware.pq_aidl-V3-ndk.so',
        'vendor/lib64/vendor.mediatek.hardware.pq_aidl-V3-ndk.so',

    ): blob_fixup()
        .replace_needed('android.hardware.graphics.common-V3-ndk.so',
                        'android.hardware.graphics.common-V7-ndk.so'),

    (
        'vendor/lib64/libcodec2_vpp_AIMEMC_plugin.so',
        'vendor/lib64/libcodec2_vpp_AISR_plugin.so',
    ): blob_fixup()
        .replace_needed('android.hardware.graphics.allocator-V1-ndk.so',
                        'android.hardware.graphics.allocator-V2-ndk.so')
        .replace_needed('android.hardware.graphics.common-V3-ndk.so',
                        'android.hardware.graphics.common-V7-ndk.so'),

    (
        'system_ext/lib/vendor.mediatek.hardware.camera.isphal-V1-ndk.so',
        'system_ext/lib64/vendor.mediatek.hardware.camera.isphal-V1-ndk.so',
    ): blob_fixup()
        .replace_needed('android.hardware.graphics.common-V6-ndk.so',
                        'android.hardware.graphics.common-V7-ndk.so'),

    'vendor/bin/hw/android.hardware.graphics.composer@3.1-service': blob_fixup()
        .replace_needed('android.hardware.graphics.composer@2.1-resources.so',
                        'android.hardware.graphics.composer@2.1-resources-v34.so'),

    (
        'odm/lib/hw/displayfeature.default.so',
        'odm/lib64/hw/displayfeature.default.so',
        'vendor/bin/dconfig',
        'vendor/lib/hw/vendor.mediatek.hardware.pq_aidl-impl.so',
        'vendor/lib64/hw/vendor.mediatek.hardware.pq_aidl-impl.so',
        'vendor/lib/libmmlpqImpl.so',
        'vendor/lib64/libmmlpqImpl.so',
        'vendor/lib/libpqxmlparser.so',
        'vendor/lib64/libpqxmlparser.so',
    ): blob_fixup()
        .replace_needed('libtinyxml2.so', 'libtinyxml2-v34.so'),

    'vendor/lib64/libcom.xiaomi.grallocutils.so': blob_fixup()
        .add_needed('libui_shim.so'),

    ('vendor/lib64/libalAILDC.so', 'vendor/lib64/libalLDC.so', 'vendor/lib64/libalhLDC.so'): blob_fixup()
        .clear_symbol_version('AHardwareBuffer_allocate')
        .clear_symbol_version('AHardwareBuffer_describe')
        .clear_symbol_version('AHardwareBuffer_lock')
        .clear_symbol_version('AHardwareBuffer_release')
        .clear_symbol_version('AHardwareBuffer_unlock'),

    'vendor/lib64/libneuralnetworks_sl_driver_mtk_prebuilt.so': blob_fixup()
        .clear_symbol_version('AHardwareBuffer_allocate')
        .clear_symbol_version('AHardwareBuffer_createFromHandle')
        .clear_symbol_version('AHardwareBuffer_describe')
        .clear_symbol_version('AHardwareBuffer_getNativeHandle')
        .clear_symbol_version('AHardwareBuffer_lock')
        .clear_symbol_version('AHardwareBuffer_release')
        .clear_symbol_version('AHardwareBuffer_unlock'),
    
    'vendor/lib64/libMiPhotoFilter.so': blob_fixup()
        .clear_symbol_version('AHardwareBuffer_allocate')
        .clear_symbol_version('AHardwareBuffer_describe')
        .clear_symbol_version('AHardwareBuffer_isSupported')
        .clear_symbol_version('AHardwareBuffer_lockPlanes')
        .clear_symbol_version('AHardwareBuffer_release')
        .clear_symbol_version('AHardwareBuffer_unlock'),

    (
        'vendor/lib64/libmialgoengine.so',
        'vendor/lib64/libcam.hal3a.so',
        'vendor/lib64/libcam.hal3a.ctrl.so',
        'vendor/lib64/hw/hwcomposer.mtk_common.so',
        'vendor/lib64/libmtkcam_cputrack.so',
        'vendor/lib64/libmtkcam_request_requlator.so'
    ): blob_fixup()
        .add_needed('libprocessgroup_shim.so'),

    (
        'vendor/lib64/libnvram.so',
        'vendor/bin/hw/android.hardware.usb-aidl-service.mediatekv1.0',
        'odm/lib64/tms-utils.so',
        'odm/lib64/nfc_nci.thn31nfc.tms.so',
        'odm/lib64/nfc_nci.nqx.default.hw.so'
    ): blob_fixup()
        .add_needed('libbase_shim.so'),

    'vendor/bin/mnld': blob_fixup()
        .replace_needed('libmnl.so', 'libmnl-mtk.so'),

    'odm/lib64/hw/vendor.xiaomi.sensor.citsensorservice@2.0-impl.so': blob_fixup()
        .add_needed('libui_shim.so')
        .replace_needed('libtinyxml2.so', 'libtinyxml2-v34.so'),

    ('vendor/lib/hw/audio.primary.mediatek.so',
     'vendor/lib64/hw/audio.primary.mediatek.so'): blob_fixup()
        .replace_needed('libalsautils.so', 'libalsautils-v33.so')
        .replace_needed('libtinyxml2.so', 'libtinyxml2-v34.so'),

    'vendor/bin/hw/mtkfusionrild': blob_fixup()
        .add_needed('libutils-v33.so'),
}

module = ExtractUtilsModule(
    'zircon',
    'xiaomi',
    lib_fixups=lib_fixups,
    blob_fixups=blob_fixups,
    namespace_imports=namespace_imports,
)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
