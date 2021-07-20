/*
 * fs_op_chmodfile.h
 *
 * description: change permissions for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#ifndef FS_OP_CHMODFILE_H_
#define FS_OP_CHMODFILE_H_

#include <sys/stat.h>
#include "fs_util_volume.h"

/**
 * Change the permissions.
 *
 * @param fs the file system
 * @param file_ino the inode number
 * @param perms the permissions
 * @param 0 if successful
 */
int fs_chmod(struct fs_ext2 *fs, int file_ino, int perms);

#endif /* FS_OP_CHMODFILE_H_ */
