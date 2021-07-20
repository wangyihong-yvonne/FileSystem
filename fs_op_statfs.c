/*
 * fs_op_statvfs.c
 *
 * description: file system volume status
 * for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "fs_op_statfs.h"
#include "fs_dev_blkdev.h"

/**
 * Return information about a file system volume
 * in the buffer pointed to by sb.
 *
 * @param fs the file system
 * @param sb statvfs buffer
 */
void fs_statfs(struct fs_ext2 *fs, struct statvfs *sb)
{
    // initialize fields to 0
    memset(sb, 0, sizeof(struct statvfs));

    // Set the following fields from volume info and constants
    //  f_bsize:	fundamental file system block size
    //  f_blocks:	total blocks in file system
    //  f_bfree:	free blocks in file system (computed from free list)
    //  f_bavail	free blocks available to non-superuser (same as b_free)
    //  f_files:	total file nodes in file system
    //  f_ffree:    total free file nodes (computed from free list)
    //  f_favail:	total free file nodes available (same as ffree to non-superuser)
    //  f_namemax:	maximum length of file name (not including null terminator)

    // compute number of free blocks
    int n_blocks_free = 0;
    for (int i = 0; i < fs->n_blocks; i++) {
        if (FD_ISSET(i, fs->block_map) == 0) {
            n_blocks_free++;
        }
    }

    // compute number of free inodes
    int n_inodes_free = 0;
    for (int i = 0; i < fs->n_inodes; i++) {
        if (FD_ISSET(i, fs->inode_map) == 0) {
            n_inodes_free++;
        }
    }

    sb->f_bsize = FS_BLOCK_SIZE;
    sb->f_blocks = fs->n_blocks;
    sb->f_bfree = n_blocks_free;
    sb->f_bavail = sb->f_bfree;
    sb->f_files = fs->n_inodes;
    sb->f_ffree = n_inodes_free;
    sb->f_favail = sb->f_ffree;
    sb->f_namemax = FS_FILENAME_SIZE-1;
}
