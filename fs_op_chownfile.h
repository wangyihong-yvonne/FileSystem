/*
 * fs_op_chownfile.h
 *
 * description: change owner/group for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#ifndef FS_OP_CHOWNFILE_H_
#define FS_OP_CHOWNFILE_H_

#include <sys/stat.h>
#include "fs_util_volume.h"

/**
 * Change the owner and group.
 *
 * @param fs the file system
 * @param file_ino the inode number
 * @param owner the owner id
 * @param group the group id
 * @return 0 if successful
 */
int fs_chown(struct fs_ext2 *fs, int file_ino, int owner, int group);

#endif /* FS_OP_CHOWNFILE_H_ */
