# Android device tree for Xiaomi devices with MSM89xx SoC running mainline kernel

## Kernel repositories

|                   Target                  |               Path               |                     URL                     |         Branch         |
|-------------------------------------------|----------------------------------|---------------------------------------------|------------------------|
| mi8916                                    | kernel/mainline/msm8916-mainline | https://github.com/msm8916-mainline/linux   | wip/msm8916/6.19       |
| mi8953_a, daisy_mainline, tissot_mainline | kernel/mainline/msm8953-mainline | https://github.com/msm8953-mainline/linux   | 7.1.3/main             |
| mi8956                                    | kernel/mainline/msm8956-mainline | https://codeberg.org/msm8956-mainline/linux | msm8956/7.2.3          |
| mi8998                                    | kernel/mainline/msm8998-mainline | https://github.com/msm8998-mainline/linux   | qcom-msm8998-6.1.y     |
| mi439_mainline, mi89x7, tiare_mainline    | kernel/mainline/msm89x7-mainline | https://github.com/msm89x7-mainline/linux   | msm89x7/7.1.3          |

## Patches

Apply the shared patches listed in
`device/mainline/common/docs/KERNEL_PATCHES.md`.

This includes the Android platform patch, the `MEMFD_ASHMEM_SHIM` Kconfig edit
and the patches for Linux v6.1.

Device specific:

| Commit name | Purpose | Source |
|-------------|---------|--------|
| `[HACK] input: ft5x06: Force input device name to "focaltech"` | Workaround for ts_vkeys configuration | [Here](./.kernel-patches/0001-HACK-input-ft5x06-Force-input-device-name-to-focalte.patch) |

## Notes for `mi439_mainline` target
- If you're flashing the device starting from stock OS, please firstly flash vbmeta image from any working custom ROM (Example: [Official LineageOS builds](https://download.lineageos.org/devices/Mi439/builds)). We do not enable AVB support here for smaller image sizes, for now.
- If you're flashing the device starting from non mainline linux OS, please flash the appropriate dtbo image from [dtbo-lk2nd](https://github.com/barni2000/dtbo-lk2nd/releases).
- The target only supports Xiaomi SDM439 devices with LM3697 backlight IC. KTD3137 backlight IC isn't supported in the kernel yet.
- Almost all of the touchscreen variants aren't supported on in the kernel yet. You'll have to interact with the device in other ways.

## Notes for `mi8916` target

- The target may boot only on Redmi 2 (`wt88047`) as of now.
- The target uses SD card for `/data` mountpoint. Make sure to have a SD card with 2 GB size at minimum inserted.

## Notes for `tiare_mainline` target

- The device has `recovery` partition sized at 25 MB which is too small to hold `recovery.img` built from the target. To boot the `recovery.img`, flash the image to `boot` partition and continue with normal boot.
- The target is 32-bit, which requires the following changes to be made in AOSP tree in order to boot:
  1. On `packages/modules/Connectivity/bpf/loader/NetBpfLoad.cpp`, locate to the next line of the line containing `[Arm] 64-bit userspace required on 6.2+ kernels (%d).` (which should be `return 1;`), remove the line.
  2. On `system/netd/server/XfrmController.cpp`, in function `validateResponse`, replace the `return` statement containing `Error netlink message` with `return netdutils::status::ok;`.

## Notes for `mi8998` target

- Display and Graphics does not work properly for now. Use `scrcpy` tool to interact.
