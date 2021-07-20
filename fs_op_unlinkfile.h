/*
 * fs_op_unlinkfile.h
 *
 * description: unlink a file or empty subdirectory
 * in a directory for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#ifndef FS_OP_UNLINKFILE_H_
#define FS_OP_UNLINKFILE_H_

#include "fs_util_volume.h"

/**
 * Unlink file or empty subdirectory from a directory.
 *
 * Errors
 *   -ENOTDIR  - dir_ino not a directory
 *   -ENOTEMPTY - file_ino subdirectory not empty
 *   -EPERM    - not allowed
 *   -ENOENT   - file_ino not child of dir_ino
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param dir_ino inode of parent directory
 * @param name name of the file or subdirectory in the directory
 * @return 0 if successful, -error if cannot create
 */
int fs_unlinkat(struct fs_ext2 *fs, int dir_ino, const char *name);

/**
 * Unlink file from a directory.
 *
 * Errors
 *   -EISDIR   - file_ino is a directory
 *   -ENOTDIR  - dir_ino not a directory
 *   -EPERM    - not allowed
 *   -ENOENT   - file_ino not child of dir_ino
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param dir_ino inode of parent directory
 * @param name name of the file in the directory
 * @return 0 if successful, -error if cannot create
 */
int fs_unlinkfile(struct fs_ext2 *fs, int dir_ino, const char *name);

/**
 * Remove subdirectory from a directory.
 *
 * Errors
 *   -ENOTDIR  - dir_ino not a directory
 *   -ENOTDIR  - file_ino not a directory
 *   -ENOTEMPTY - subdirectory not empty
 *   -EPERM    - not allowed
 *   -ENOENT   - file_ino not child of dir_ino
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param dir_ino inode of parent directory
 * @param name name of the subdirectory in the directory
 * @return 0 if successful, -error if cannot create
 */
int fs_rmdir(struct fs_ext2 *fs, int dir_ino, const char *name) ;

#endif /* FS_OP_UNLINKFILE_H_ */
