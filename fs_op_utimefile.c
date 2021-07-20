/*
 * fs_op_utimefile.c
 *
 * description: change mod time for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#include <stdlib.h>
#include <time.h>

#include "fs_op_utimefile.h"

/**
 * Change the modification time.
 *
 * @param fs the file system
 * @param file_ino the inode number
 * @param mod_time the modification time;
 * @return 0 if successful
 */
int fs_utime(struct fs_ext2 *fs, int file_ino, time_t mod_time)
{
    fs->inodes[file_ino].mtime =  mod_time;

    fs_mark_inode(fs, file_ino);  // mark inode changed
    fs_sync_metadata(fs); // sync changed inode

    return 0;
}
