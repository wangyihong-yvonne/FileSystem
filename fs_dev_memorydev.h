/*
 * fs_dev_memorydev.h
 *
 * description: in-memory block device creation function
 * for CS 7600 / CS 5600 file system
 *
 * Peter Desnoyers, Northeastern Computer Science, 2015
 * Philip Gust, Northeastern Computer Science, 2019
 */

#ifndef FS_DEV_MEMORYDEV_H_
#define FS_DEV_MEMORYDEV_H_

#include "fs_dev_blkdev.h"

/**
 * Create an image block device reading from a specified image file.
 *
 * @param nblks the number of blocks for the device
 * @return the block device or NULL if cannot create the block device
 */
extern struct fs_dev_blkdev *memory_blkdev_create(size_t nblks);


#endif /* FS_DEV_MEMORYDEV_H_ */
