/*
 * fs_op_statfile.h
 *
 * description: file status for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#ifndef FS_OP_STATFILE_H_
#define FS_OP_STATFILE_H_

#include <sys/stat.h>
#include "fs_util_volume.h"

/**
 * Return information about a file in the
 * buffer pointed to by sb.
 * <p>
 * Notes: The st_blocks field is number of
 * 512-byte blocks rounded up to nearest
 * 512-byte block. The st_atime field is
 * the same value as the st_mtime field
 * since last access time is not recorded
 * in struct fs_inode.
 *
 * @param fs the file system
 * @param file_ino the inode number
 * @param sb stat buffer
 */
void fs_stat(struct fs_ext2 *fs, int file_ino, struct stat *sb);

#endif /* FS_OP_STATFILE_H_ */
