/*
 * fs_op_utimefile.h
 *
 * description: change mod time for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#ifndef FS_OP_UTIMEFILE_H_
#define FS_OP_UTIMEFILE_H_

#include <time.h>
#include "fs_util_volume.h"

/**
 * Change the modification time.
 *
 * @param fs the file system
 * @param file_ino the inode number
 * @param mod_time the modification time;
 * @return 0 if successful
 */
int fs_utime(struct fs_ext2 *fs, int file_ino, time_t mod_time);

#endif /* FS_OP_UTIMEFILE_H_ */
