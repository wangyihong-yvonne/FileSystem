/*
 * fs_op_chownfile.c
 *
 * description: change owner/group for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "fs_op_chownfile.h"

/**
 * Change the owner and group.
 *
 * @param fs the file system
 * @param file_ino the inode number
 * @param owner the owner id
 * @param group the group id
 * @return 0 if successful
 */
int fs_chown(struct fs_ext2 *fs, int file_ino, int owner, int group)
{
    // set owner and group mask
    fs->inodes[file_ino].uid = owner;
    fs->inodes[file_ino].gid = group;

    fs_mark_inode(fs, file_ino);  // mark inode changed
    fs_sync_metadata(fs); // sync changed inode

    return 0;
}
