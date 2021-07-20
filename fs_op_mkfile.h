/*
 * fs_op_makefile.h
 *
 * description: make a file, dir, or link in a directory
 * for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#ifndef FS_OP_MKFILE_H_
#define FS_OP_MKFILE_H_

#include <fcntl.h>
#include "fs_util_volume.h"

/**
 * Make file in a directory.
 *
 * Errors
 *   -ENAMETOOLONG  - name too long
 *   -ENOTDIR  - dir_ino not a directory
 *   -EEXIST   - entry already exists
 *   -ENOSPC   - free entry or block not found
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param dir_ino inode of parent directory
 * @param name the file name
 * @param mode creation mode
 * @return inode of file if successful, -error if cannot create
 */
int fs_mkfile(struct fs_ext2 *fs, int dir_ino, const char* name, mode_t mode);

/**
 * Make subdirectory in a directory.
 *
 * Errors
 *   -ENAMETOOLONG  - name too long
 *   -ENOTDIR  - dir_ino not a directory
 *   -EEXIST   - entry already exists
 *   -ENoSPC   - free entry or block not found
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param dir_ino inode of parent directory
 * @param name the subdirectory name
 * @param mode creation mode
 * @return inode of file if successful, -error if cannot create
 */
int fs_mkdir(struct fs_ext2 *fs, int dir_ino, const char* name, mode_t mode);

/**
 * Make link to existing non-directory inode in a directory.
 *
 * Errors
 *   -ENAMETOOLONG  - name too long
 *   -ENOTDIR  - dir_ino not a directory
 *   -EISDIR   - link file inode is directory
 *   -EEXIST   - entry already exists
 *   -ENOSPC   - free entry or block not found
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param dir_ino inode of parent directory
 * @param file_ino inode of existing file inode
 * @param name the file name
 * @return inode of file if successful, -error if cannot link
 */
int fs_mklink(struct fs_ext2 *fs, int dir_ino, int file_ino, const char* name);

#endif /* FS_OP_MKFILE_H_ */
