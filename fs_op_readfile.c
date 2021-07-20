/*
 * fs_op_readfile.c
 *
 * description: read, pread file for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "fs_op_readfile.h"
#include "fs_dev_blkdev.h"

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
int fs_preadfile(struct fs_ext2 *fs, int file_ino, void *content, int n_bytes, int offset)
{
    // ensure file_ino is a regular file
    if (!S_ISREG(fs->inodes[file_ino].mode)) {
        return -EISDIR;
    }

    // get block number of first file block
    int file_blkno = fs->inodes[file_ino].direct[0];
    if (file_blkno == 0) {
        return -ENOSPC;  // no file block
    }

    // compute number of bytes to read
    int n_avail = fs->inodes[file_ino].size - offset;
    int n_read =  (n_bytes < n_avail) ? n_bytes : n_avail;
    if ((n_bytes <= 0) || (offset < 0) || (n_read <= 0)) {
        return 0;
    }

    // read file block from disk
    block file_blk;
    if (fs->dev->ops->read(fs->dev, file_blkno, 1, file_blk) != SUCCESS) {
        return -EIO;
    }

    // copy block to contents
    memcpy(content, file_blk + offset, n_read);

    return n_read;  // success
}


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
int fs_readfile(struct fs_ext2 *fs, int file_ino, void *content, int n_bytes)
{
    return fs_preadfile(fs, file_ino, content, n_bytes, 0);
}