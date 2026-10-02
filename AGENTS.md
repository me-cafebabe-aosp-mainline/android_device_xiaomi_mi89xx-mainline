# AGENTS.md - device/xiaomi/mi89xx-mainline

Agents must read this file before touching anything in this repository.

Part of the mainline repository set. Map of all repos: `vendor/mainline/docs/REPOSITORIES.md`.

Family tree for Xiaomi MSM89xx devices. One directory per device, and
several kernel forks (see `README.md`).

## Read first

| Task | Read |
|------|------|
| Anything about bringup, kernel, boot, init, SELinux | `device/mainline/common/docs/README.md` |
| Commits, style, review | `hardware/mainline/common/docs/` |
| Qualcomm layers | `device/mainline/qcom-common/docs/NEW_DEVICE.md` |
| Blob extraction | `extract-files.py`, `setup-makefiles.py`, `proprietary-files.txt` |

## Hard rules

- Do not build, flash or run tests; the human does and reports back.
- Do not search from the AOSP tree root; use precise directories.
- Keep device specific values here. Things that apply to more devices
  belong in the common trees (ask first).
- Do not copy the shared kernel patch table into the README; link to
  `device/mainline/common/docs/KERNEL_PATCHES.md`.
- Do not add firmware or proprietary files.
