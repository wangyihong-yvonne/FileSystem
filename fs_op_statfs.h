/*
 * fs_op_statvfs.h
 *
 * description: file system volume status
 * for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#ifndef FS_OP_STATFS_H_
#define FS_OP_STATFS_H_

#include <sys/statvfs.h>
#include "fs_util_volume.h"

/**
 * Return information about a file system volume
 * in the buffer pointed to by sb.
 *
 * @param fs the file system
 * @param sb statvfs buffer
 */
void fs_statfs(struct fs_ext2 *fs, struct statvfs *sb);

#endif /* FS_OP_STATFS_H_ */
