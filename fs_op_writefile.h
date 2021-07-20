/*
 * fs_op_writefile.h
 *
 * description: write, pwrite file for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#ifndef FS_OP_WRITEFILE_H_
#define FS_OP_WRITEFILE_H_

#include "fs_util_volume.h"

/**
 * Write contents to a file starting at a file offset.
 *
 * Note: current implementation supports
 * only 1 block of content
 *
 * Errors
 *   -ENISDIR  - dir_ino not a directory
 *   -ENOSPC   - free entry or block not found
 *   -EFBIG    - content too large
 *   -EINVAL   - invalid n_bytes or off
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param file_ino inode of file
 * @param content the content
 * @param n_bytes number of bytes
 * @param offset offset of initial byte
 * @return 0 if successful, -error if error occurred
 */
int fs_pwritefile(struct fs_ext2 *fs, int file_ino, const void *content, int n_bytes, int offset);

/**
 * Write contents to a file.
 *
 * Note: current implementation supports
 * only 1 block of content
 *
 * Errors
 *   -ENISDIR  - dir_ino not a directory
 *   -ENOSPC   - free entry or block not found
 *   -EFBIG    - content too large
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param file_ino inode of file
 * @param content the content
 * @param n_bytes number of bytes
 * @return 0 if successful, -error if error occurred
 */
int fs_writefile(struct fs_ext2 *fs, int file_ino, const void *content, int n_bytes);

#endif /* FS_OP_WRITEFILE_H_ */
