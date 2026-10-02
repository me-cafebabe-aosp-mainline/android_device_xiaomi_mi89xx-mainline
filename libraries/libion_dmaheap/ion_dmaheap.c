/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Drop-in replacement for the legacy libion.so, backed by dma-buf heaps.
 *
 * Vendor blobs built for msm kernels allocate through libion from msm ION
 * heaps selected by a heap ID mask. Mainline kernels have no ION; this
 * library translates those requests to allocations from /dev/dma_heap.
 *
 * ION handles are not a thing with dma-buf heaps, so the handle based part
 * of the API uses the dma-buf fd itself as the handle.
 *
 * Blobs issuing the legacy ION ioctls themselves are served by ion_legacy.c.
 */

#define LOG_TAG "ion_dmaheap"

#include <errno.h>
#include <fcntl.h>
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <stddef.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <log/log.h>

typedef int ion_user_handle_t;

#define DMA_HEAP_DIR "/dev/dma_heap"

/* msm ION heap IDs, from the downstream uapi/linux/msm_ion.h */
#define ION_SECURE_HEAP_ID 9
#define ION_QSECOM_TA_HEAP_ID 19
#define ION_SYSTEM_HEAP_ID 25
#define ION_USER_CONTIG_HEAP_ID 26
#define ION_QSECOM_HEAP_ID 27

/* Bit 31 of the flags; also the reserved bit of the heap ID mask. */
#define ION_FLAG_SECURE (1U << 31)

#define MAX_HEAP_NAMES 2

struct heap_map {
    unsigned int id;
    const char* names[MAX_HEAP_NAMES];
};

/*
 * dma-buf heaps tried for each ION heap, in order. The table is sorted by
 * descending heap ID, the order ION tries the heaps of a mask in. CMA heaps
 * are named after their reserved-memory node; "default_cma_region" is the
 * global CMA area.
 */
static const struct heap_map heap_maps[] = {
        {ION_QSECOM_HEAP_ID, {"qseecom", NULL}},
        {ION_USER_CONTIG_HEAP_ID, {"default_cma_region", NULL}},
        {ION_SYSTEM_HEAP_ID, {"system", NULL}},
        {ION_QSECOM_TA_HEAP_ID, {"qseecom_ta", "qseecom"}},
};

int ion_open() {
    int fd = open(DMA_HEAP_DIR, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (fd < 0) ALOGE("open %s failed: %s", DMA_HEAP_DIR, strerror(errno));

    return fd;
}

int ion_close(int fd) {
    int ret = close(fd);
    if (ret < 0) return -errno;
    return ret;
}

static int alloc_from_heap(int fd, const char* name, size_t len, int* handle_fd) {
    struct dma_heap_allocation_data data = {
            .len = len,
            .fd_flags = O_RDWR | O_CLOEXEC,
    };
    int heap_fd, ret;

    heap_fd = openat(fd, name, O_RDONLY | O_CLOEXEC);
    if (heap_fd < 0) return -errno;

    ret = ioctl(heap_fd, DMA_HEAP_IOCTL_ALLOC, &data);
    ret = ret < 0 ? -errno : 0;
    close(heap_fd);
    if (ret < 0) return ret;

    *handle_fd = data.fd;
    return 0;
}

int ion_alloc_fd(int fd, size_t len, size_t align, unsigned int heap_mask, unsigned int flags,
                 int* handle_fd) {
    int ret = -ENODEV;
    size_t i, j;

    (void)align; /* dma-buf heaps align allocations by themselves */

    if (handle_fd == NULL) return -EINVAL;

    if ((flags & ION_FLAG_SECURE) || (heap_mask & (1U << ION_SECURE_HEAP_ID))) {
        ALOGE("secure allocations are not supported (heap_mask 0x%x, flags 0x%x)", heap_mask,
              flags);
        return -EOPNOTSUPP;
    }

    /* ION tries the heaps of the mask in order of descending ID. */
    for (i = 0; i < sizeof(heap_maps) / sizeof(heap_maps[0]); i++) {
        if (!(heap_mask & (1U << heap_maps[i].id))) continue;

        for (j = 0; j < MAX_HEAP_NAMES && heap_maps[i].names[j]; j++) {
            ret = alloc_from_heap(fd, heap_maps[i].names[j], len, handle_fd);
            if (ret == 0) return 0;
            if (ret != -ENOENT)
                ALOGE("allocating %zu bytes from %s failed: %s", len, heap_maps[i].names[j],
                      strerror(-ret));
        }
    }

    ALOGE("no dma-buf heap could satisfy heap_mask 0x%x, len %zu: %s", heap_mask, len,
          strerror(-ret));
    return ret;
}

int ion_alloc(int fd, size_t len, size_t align, unsigned int heap_mask, unsigned int flags,
              ion_user_handle_t* handle) {
    if (handle == NULL) return -EINVAL;

    return ion_alloc_fd(fd, len, align, heap_mask, flags, handle);
}

int ion_free(int fd, ion_user_handle_t handle) {
    (void)fd;

    /* Handle 0 is invalid in ION, and used to probe for legacy ION. */
    if (handle <= 0) return -EINVAL;

    return ion_close(handle);
}

int ion_share(int fd, ion_user_handle_t handle, int* share_fd) {
    (void)fd;

    if (share_fd == NULL) return -EINVAL;

    *share_fd = fcntl(handle, F_DUPFD_CLOEXEC, 0);
    if (*share_fd < 0) return -errno;
    return 0;
}

int ion_import(int fd, int share_fd, ion_user_handle_t* handle) {
    return ion_share(fd, share_fd, handle);
}

int ion_map(int fd, ion_user_handle_t handle, size_t length, int prot, int flags, off_t offset,
            unsigned char** ptr, int* map_fd) {
    unsigned char* tmp_ptr;
    int ret;

    if (map_fd == NULL || ptr == NULL) return -EINVAL;

    ret = ion_share(fd, handle, map_fd);
    if (ret < 0) return ret;

    tmp_ptr = mmap(NULL, length, prot, flags, *map_fd, offset);
    if (tmp_ptr == MAP_FAILED) {
        ret = -errno;
        ALOGE("mmap failed: %s", strerror(-ret));
        close(*map_fd);
        *map_fd = -1;
        return ret;
    }

    *ptr = tmp_ptr;
    return 0;
}

int ion_sync_fd(int fd, int handle_fd) {
    struct dma_buf_sync sync = {
            .flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW,
    };

    (void)fd;

    if (ioctl(handle_fd, DMA_BUF_IOCTL_SYNC, &sync) < 0) return -errno;

    sync.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_RW;
    if (ioctl(handle_fd, DMA_BUF_IOCTL_SYNC, &sync) < 0) return -errno;

    return 0;
}

int ion_query_heap_cnt(int fd, int* cnt) {
    (void)fd;
    (void)cnt;

    return -ENOTTY;
}

int ion_query_get_heaps(int fd, int cnt, void* buffers) {
    (void)fd;
    (void)cnt;
    (void)buffers;

    return -ENOTTY;
}

int ion_is_legacy(int fd) {
    (void)fd;

    /* Clients only use the handle based API on legacy ION. */
    return 1;
}

int ion_is_using_modular_heaps(int fd) {
    (void)fd;

    return 0;
}
