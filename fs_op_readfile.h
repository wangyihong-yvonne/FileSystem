/*
 * fs_op_readfile.h
 *
 * description: read, pread file for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#ifndef FS_OP_READFILE_H_
#define FS_OP_READFILE_H_

#include "fs_util_volume.h"

/**
 * Read contents from a file starting at a file offset.
 *
 * Errors
 *   -ENISDIR  - file_ino is a directory
 *   -ENOSPC   - block not found
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param file_ino inode of file
 * @param content the returned content
 * @param n_bytes number of bytes to read
 * @param offset offset of initial byte
 * @return number of bytes read if successful, -error if error occurred
 */
int fs_preadfile(struct fs_ext2 *fs, int file_ino, void *content, int n_bytes, int offset);

/**
 * Read contents from a file.
 *
 * Errors
 *   -ENISDIR  - file_ino is a directory
 *   -ENOSPC   - block not found
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param file_ino inode of file
 * @param content the returned content
 * @param n_bytes number of bytes to read
 * @return number of bytes read if successful, -error if error occurred
 */
int fs_readfile(struct fs_ext2 *fs, int file_ino, void *content, int n_bytes);

#endif /* FS_OP_READFILE_H_ */
