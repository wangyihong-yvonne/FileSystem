/*
 * fs_op_writefile.h
 *
 * description: truncate file for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#ifndef FS_OP_TRUNCFILE_H_
#define FS_OP_TRUNCFILE_H_

#include "fs_util_volume.h"

/**
 * Cause the file to be truncated (or extended)
 * to n_bytes bytes in size. If the file size
 * exceeds length, any extra data is discarded.
 * If the file size is smaller than length, the
 * file is extended and filled with zeros to the
 * indicated length.
 *
 * Note: current implementation supports
 * only 1 block of content
 *
 * Errors
 *   -EISDIR   - file_in is a directory
 *   -ENOSPC   - free entry or block not found
 *   -EFBIG    - content too large
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param file_ino inode of file
 * @param n_bytes number of bytes
 * @return 0 if successful, -error if error occurred
 */
int fs_truncfile(struct fs_ext2 *fs, int file_ino, int n_bytes);

#endif /* FS_OP_TRUNCFILE_H_ */
