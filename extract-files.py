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
}  # fmt: skip

namespace_imports = [
    'hardware/mainline/qcom',
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
