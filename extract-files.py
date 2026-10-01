#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)
from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)

blob_fixups: blob_fixups_user_type = {
    (
        'vendor/lib64/libQSEEComAPI.so',
        'vendor/lib64/libsecureui.so',
    ): blob_fixup()
        .replace_needed('libion.so', 'libion_dmaheap.so'),
    (
        'vendor/lib64/libgf_algo.so',
        'vendor/lib64/libgf_ca.so',
        'vendor/lib64/libgf_hal.so',
    ): blob_fixup()
        .replace_needed('libstdc++.so', 'libstdc++_vendor.so'),
    (
        'vendor/lib64/libvendor.goodix.hardware.fingerprint@1.0.so',
    ): blob_fixup()
        .add_needed('libhidlbase_shim.so'),
    (
        'vendor/lib64/libvendor.goodix.hardware.fingerprint@1.0-service.so',
    ): blob_fixup()
        .remove_needed('libprotobuf-cpp-lite.so'),
    (
        'vendor/lib64/hw/gf_fingerprint.default.so',
    ): blob_fixup()
        .fix_soname(),
}  # fmt: skip

namespace_imports = [
    'device/xiaomi/mi89xx-mainline',
    'vendor/qcom/opensource/display',
]

module = ExtractUtilsModule(
    'mi89xx-mainline',
    'xiaomi',
    blob_fixups=blob_fixups,
    namespace_imports=namespace_imports,
)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
