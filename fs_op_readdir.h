/*
 * fs_op_readdir.h
 *
 * description: read file for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#ifndef FS_OP_READDIR_H_
#define FS_OP_READDIR_H_

#include "fs_util_volume.h"

/** struct for a directory stream */
typedef struct FS_DIR FS_DIR;

/**
 * Opens a directory stream corresponding to the
 * directory inode, and returns a pointer to the
 * directory stream. The stream is positioned at
 * the first entry in the directory.
 *
 * @param fs the file system
 * @param dir_ino inode of directory
 * @return pointer to directory stream or NULL if error
 */
FS_DIR* fs_opendir(struct fs_ext2 *fs, int dir_ino);

/**
 * Closes the directory stream associated with dirp.
 *
 * @param dirp the directory stream
 */
void fs_closedir(FS_DIR *dirp);

/**
 * Returns a pointer to a fs_dirent structure
 * representing the next directory entry in the
 * directory stream pointed to by dirp. Returns
 * NULL on  reaching the end of the directory
 * stream or if an error occurred.
 *
 * @param dirp pointer to a directory stream
 * @return next entry or NULL if end of stream or error
 */
struct fs_dirent *fs_readdir(FS_DIR *dirp);

#endif /* FS_OP_READDIR_H_ */
