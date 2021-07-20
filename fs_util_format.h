/*
 * fs_util_format.h
 *
 * description: format a volume file system
 * for CS 7600 / CS 5600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Peter Desnoyers,  November 2016
 * Philip Gust, March 2021
 */

#ifndef FS_UTIL_FORMAT_H_
#define FS_UTIL_FORMAT_H_

#include "fs_dev_blkdev.h"
#include "fs_util_volume.h"

/**
 * Format a file system volume for block device.
 * New volume occupies entire block device.
 *
 * Errors
 *   -EIO      - i/o error
 *
 * @param dev the block device
 * @param fold_case fold case for directory entries
 * @param ignore_case ignore case for directory entries
 * @return status 0 for success, -error for errors
 */
int fs_format_volume(struct fs_dev_blkdev* dev, int ignore_case, int fold_case);

#endif /* FS_UTIL_FORMAT_H_ */
