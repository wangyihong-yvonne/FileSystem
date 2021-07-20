/*
 * fs_op_chmodfile.c
 *
 * description: change permissions for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#include <stdlib.h>
#include <time.h>

#include "fs_op_chmodfile.h"

/**
 * Change the permissions.
 *
 * @param fs the file system
 * @param file_ino the inode number
 * @param perms the permissions
 * @param 0 if successful
 */
int fs_chmod(struct fs_ext2 *fs, int file_ino, int perms)
{
    // permissions mask
    static const int perm_msk = S_IRWXU | S_IRWXG | S_IRWXO;

    fs->inodes[file_ino].mode =
          (fs->inodes[file_ino].mode & ~perm_msk)  // clear permissions
        | (perms & perm_msk); // set new permissions

    fs_mark_inode(fs, file_ino);  // mark inode changed
    fs_sync_metadata(fs); // sync changed inode

    return 0;
}
