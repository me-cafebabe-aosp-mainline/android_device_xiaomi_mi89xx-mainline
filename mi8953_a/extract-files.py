#!/usr/bin/env -S PYTHONPATH=../../../../tools/extract-utils python3
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
        'odm/lib64/libvendor.goodix.hardware.fingerprint@1.0-service.so',
    ): blob_fixup()
        .remove_needed('libprotobuf-cpp-lite.so'),
}  # fmt: skip

namespace_imports = [
    'vendor/xiaomi/mi89xx-mainline',
]

module = ExtractUtilsModule(
    'mi8953_a',
    'xiaomi',
    blob_fixups=blob_fixups,
    namespace_imports=namespace_imports,
)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
