/*
 * fs_dev_memorydev.c
 *
 * description: in-memory block device functions
 * for CS 7600 / CS 5600 file system
 *
 * Peter Desnoyers, Northeastern Computer Science, 2011
 * Philip Gust, Northeastern Computer Science, 2019
 */

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "fs_dev_blkdev.h"

/** Definition of memory block device */
struct memory_dev {
    block* blocks;	// pointer to block memory
    int   nblks;	// number of blocks in device
};


/**
 * The number of blocks in the block device.
 *
 * @param the block device
 */
static int memdev_num_blocks(struct fs_dev_blkdev *dev)
{
    struct memory_dev *pvt = dev->private;
    return pvt->nblks;
}

/**
 * Read blocks from block device starting at give block offset.
 *
 * @param dev the block device
 * @param offset starting block offset
 * @param len number of blocks to read
 * @param buf the input buffer
 * @return SUCCESS if successful,
 *  E_UNAVAIL if device unavailable, E_SIZE if out of range
 */
static int memdev_read(struct fs_dev_blkdev *dev, int offset, int len, void *buf)
{
    struct memory_dev *pvt = dev->private;

    /* to fail we free its memory and set it to NULL */
    if (pvt->blocks == NULL) {
        return E_UNAVAIL;
    }

    if (offset >= 0 && offset+len <= pvt->nblks) {
        // copy blocks to buf
        memcpy(buf, pvt->blocks + offset, len * BLOCK_SIZE);
        return SUCCESS;
    }
    return E_SIZE;
}

/**
 * Write blocks to block device starting at give block offset.
 *
 * @param dev the block device
 * @param offset starting block offset
 * @param len number of blocks to write
 * @param buf the output buffer
 * @return SUCCESS if successful,
 *  E_UNAVAIL if device unavailable, E_SIZE if out of range
 */
static int memdev_write(struct fs_dev_blkdev * dev, int offset, int len, void *buf) {
    struct memory_dev *pvt = dev->private;

    /* to fail we free its memory and set it to NULL */
    if (pvt->blocks == NULL) {
        return E_UNAVAIL;
    }

    if (offset >= 0 && offset+len <= pvt->nblks) {
        // copy buf to blocks
        memcpy(pvt->blocks + offset, buf, len * BLOCK_SIZE);
        return SUCCESS;
    }
    return E_SIZE;
}

/**
 * Flush the block device.
 *
 * @param dev the block device
 * @aparam offset starting block offset
 * @param len number of blocks to flush
 * @return SUCCESS if successful, E_UNAVAIL if device unavailable
 */
static int memdev_flush(struct fs_dev_blkdev * dev, int offset, int len)
{
    return SUCCESS;
}

/**
 * Close the block device. After this any further
 * access to that device will fail.
 *
 * @param dev the block device
 */
static void memdev_close(struct fs_dev_blkdev *dev)
{
    struct memory_dev *pvt = dev->private;

    free(pvt->blocks);  // free storage for blocks
    pvt->blocks = NULL;
    pvt->nblks = 0;

    free(dev->private);  // free private storage
    dev->private = NULL; // crash any attempts to access
    free(dev);
}

/** Operations on this block device */
static struct blkdev_ops memdev_ops = {
    .num_blocks = memdev_num_blocks,
    .read = memdev_read,
    .write = memdev_write,
    .flush = memdev_flush,
    .close = memdev_close
};

/**
 * Create an in-memory block device.
 *
 * @param nblks number of blocks for device
 * @return the block device or NULL if cannot create
 */
struct fs_dev_blkdev *memory_blkdev_create(int nblks)
{
    struct fs_dev_blkdev *dev = malloc(sizeof(struct fs_dev_blkdev));
    struct memory_dev *pvt = malloc(sizeof(struct memory_dev));
    pvt->blocks = calloc(nblks, BLOCK_SIZE);

    // fail if cannot allocate blocks
    if (pvt->blocks == NULL) {
        free(dev);
        free(pvt);
        return NULL;
    }
    pvt->nblks = nblks;

    dev->private = pvt;
    dev->ops = &memdev_ops;
    return dev;
}

/**
 * Force an image fs_dev_blkdev into failure. After this any
 * further access to that device will return E_UNAVAIL.
 */
void memdev_fail(struct fs_dev_blkdev *dev)
{
    struct memory_dev *pvt = dev->private;

    if (pvt->blocks != NULL) {
        free(pvt->blocks);
        pvt->blocks = NULL;
    }
}
