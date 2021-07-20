/*
 * fs_op_statfile.c
 *
 * description: file status for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#include <stdlib.h>
#include <string.h>

#include "fs_op_statfile.h"
#include "fs_dev_blkdev.h"

/**
 * Calculate highest multiple m of n
 *
 * @param n the divisor
 * @param m the dividend
 * @return quotient rounded up
 */
static inline int div_round_up(int n, int m) {
    return (n + m - 1) / m;
}

/**
 * Return information about a file in the
 * buffer pointed to by sb.
 * <p>
 * Notes: The st_blocks field is number of
 * 512-byte blocks rounded up to nearest
 * 512-byte block. The st_atime field is
 * the same value as the st_mtime field
 * since last access time is not recorded
 * in struct fs_inode.
 *
 * @param fs the file system
 * @param file_ino the inode number
 * @param sb stat buffer
 */
void fs_stat(struct fs_ext2 *fs, int file_ino, struct stat *sb) {
    // initialize fields to 0
    memset(sb, 0, sizeof(struct stat));

    // Set these fields from the inode and constants:
    // st_blksize:      optimal blocksize for I/O
    // st_ino:          file inode number
    // st_mode:         mode of file
    // st_nlink:        number of hard links
    // st_uid:          user ID of the file
    // st_gid:          group ID of the file
    // st_size:         file size, in bytes
    // st_blocks:       number of 512K size blocks
    // st_atime:        last access time of the file (same as st_mtime)
    // st_mtime:        last modify time of the file
    // st_ctime:        creation time of the file

    // point to inode for inum
    struct fs_inode *in = &fs->inodes[file_ino];
    sb->st_blksize = FS_BLOCK_SIZE;
    sb->st_ino = file_ino;
    sb->st_mode = in->mode;
    sb->st_nlink = in->nlink;
    sb->st_uid = in->uid;
    sb->st_gid = in->gid;
    sb->st_size = in->size;
    // actual number of blocks expressed as multiples of 512 byte blocks
    int n_blks = div_round_up(in->size, FS_BLOCK_SIZE);
    sb->st_blocks =  n_blks * FS_BLOCK_SIZE / 512;
    sb->st_atime = sb->st_mtime = in->mtime;
    sb->st_ctime = in->ctime;
}
