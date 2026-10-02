/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Emulation of the legacy (pre-4.12) ION ioctl ABI on /dev/ion, for blobs
 * that open the device and issue the ioctls themselves instead of using
 * libion.
 *
 * Such blobs get their open() and ioctl() imports renamed to
 * ion_legacy_open() and ion_legacy_ioctl() at extraction. Opening /dev/ion
 * then yields an fd of /dev/dma_heap, and ION ioctls on it are served by the
 * libion API of this library. Everything else is passed through.
 */

#define LOG_TAG "ion_dmaheap"

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <log/log.h>

typedef int ion_user_handle_t;

int ion_open();
int ion_alloc_fd(int fd, size_t len, size_t align, unsigned int heap_mask, unsigned int flags,
                 int* handle_fd);
int ion_free(int fd, ion_user_handle_t handle);
int ion_share(int fd, ion_user_handle_t handle, int* share_fd);
int ion_import(int fd, int share_fd, ion_user_handle_t* handle);
int ion_sync_fd(int fd, int handle_fd);

#define ION_DEV "/dev/ion"
#define DMA_HEAP_DIR "/dev/dma_heap"

#define ION_IOC_MAGIC 'I'

struct ion_legacy_allocation_data {
    size_t len;
    size_t align;
    unsigned int heap_id_mask;
    unsigned int flags;
    ion_user_handle_t handle;
};

struct ion_legacy_fd_data {
    ion_user_handle_t handle;
    int fd;
};

struct ion_legacy_handle_data {
    ion_user_handle_t handle;
};

#define ION_LEGACY_IOC_ALLOC _IOWR(ION_IOC_MAGIC, 0, struct ion_legacy_allocation_data)
#define ION_LEGACY_IOC_FREE _IOWR(ION_IOC_MAGIC, 1, struct ion_legacy_handle_data)
#define ION_LEGACY_IOC_MAP _IOWR(ION_IOC_MAGIC, 2, struct ion_legacy_fd_data)
#define ION_LEGACY_IOC_SHARE _IOWR(ION_IOC_MAGIC, 4, struct ion_legacy_fd_data)
#define ION_LEGACY_IOC_IMPORT _IOWR(ION_IOC_MAGIC, 5, struct ion_legacy_fd_data)
#define ION_LEGACY_IOC_SYNC _IOWR(ION_IOC_MAGIC, 7, struct ion_legacy_fd_data)

int ion_legacy_open(const char* path, int flags, ...) {
    mode_t mode = 0;

    if (flags & (O_CREAT | O_TMPFILE)) {
        va_list ap;

        va_start(ap, flags);
        mode = (mode_t)va_arg(ap, int);
        va_end(ap);
    }

    if (path && !strcmp(path, ION_DEV)) return ion_open();

    return open(path, flags, mode);
}

/* Whether fd is /dev/dma_heap, as returned by ion_open() */
static int is_ion_fd(int fd) {
    struct stat fd_st, dir_st;

    if (fstat(fd, &fd_st) < 0 || !S_ISDIR(fd_st.st_mode)) return 0;
    if (stat(DMA_HEAP_DIR, &dir_st) < 0) return 0;

    return fd_st.st_dev == dir_st.st_dev && fd_st.st_ino == dir_st.st_ino;
}

static int ion_legacy_ioctl_impl(int fd, int request, void* arg) {
    switch ((unsigned int)request) {
        case ION_LEGACY_IOC_ALLOC: {
            struct ion_legacy_allocation_data* data = arg;

            return ion_alloc_fd(fd, data->len, data->align, data->heap_id_mask, data->flags,
                                &data->handle);
        }
        case ION_LEGACY_IOC_FREE: {
            struct ion_legacy_handle_data* data = arg;

            return ion_free(fd, data->handle);
        }
        case ION_LEGACY_IOC_MAP:
        case ION_LEGACY_IOC_SHARE: {
            struct ion_legacy_fd_data* data = arg;

            return ion_share(fd, data->handle, &data->fd);
        }
        case ION_LEGACY_IOC_IMPORT: {
            struct ion_legacy_fd_data* data = arg;

            return ion_import(fd, data->fd, &data->handle);
        }
        case ION_LEGACY_IOC_SYNC: {
            struct ion_legacy_fd_data* data = arg;

            return ion_sync_fd(fd, data->fd);
        }
        default:
            ALOGE("unsupported ION ioctl 0x%x", (unsigned int)request);
            return -ENOTTY;
    }
}

int ion_legacy_ioctl(int fd, int request, ...) {
    va_list ap;
    void* arg;
    int ret;

    va_start(ap, request);
    arg = va_arg(ap, void*);
    va_end(ap);

    if (_IOC_TYPE((unsigned int)request) != ION_IOC_MAGIC || !is_ion_fd(fd))
        return ioctl(fd, request, arg);

    if (arg == NULL) {
        errno = EFAULT;
        return -1;
    }

    ret = ion_legacy_ioctl_impl(fd, request, arg);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }

    return 0;
}
