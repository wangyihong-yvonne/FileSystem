/**
 *
 * fs_op_writefile.c
 *
 * description: write, pwrite file for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

#include "fs_op_writefile.h"
#include "fs_dev_blkdev.h"

/**
 * Write contents to a file starting at a file offset.
 *
 * Note: current implementation supports
 * only 1 block of content
 *
 * Errors
 *   -ENISDIR  - dir_ino not a directory
 *   -ENOSPC   - free entry or block not found
 *   -EFBIG    - content too large
 *   -EINVAL   - invalid n_bytes or off
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param file_ino inode of file
 * @param content the content
 * @param n_bytes number of bytes
 * @param offset offset of initial byte
 * @return 0 if successful, -error if error occurred
 */
int fs_pwritefile(struct fs_ext2 *fs, int file_ino, const void *content, int n_bytes, int offset)
{
    // ensure file_ino is a regular file
    if (!S_ISREG(fs->inodes[file_ino].mode)) {
        return -EISDIR;
    }

    // internal flag used by fs_writefile() for truncation
    int old_size = fs->inodes[file_ino].size;
    if (offset == INT_MIN) {
        offset = 0;
        old_size = 0; // truncation for single block file
    }

    // invalid n_bytes or offset
    if ((n_bytes < 0) || (offset < 0)) {
        return -EINVAL;   // invalid spec
    }

    // nothing to write if n_bytes is 0
    if (n_bytes == 0) {
        return 0;
    }

    // ensure bytes fit in block
    int new_size = offset + n_bytes;
    if (new_size > FS_BLOCK_SIZE) {
        return -EFBIG;  // contents too large
    }

    // get block number of first file block
    int file_blkno = fs->inodes[file_ino].direct[0];
    if (file_blkno == 0) {
        return -ENOSPC;  // no file block
    }

    block file_blk;  // space for block content

    // read current block if partial overwrite
    if (old_size > 0) {  // file not empty
        if ((offset > 0) || (new_size < old_size)) {
            if (fs->dev->ops->read(fs->dev, file_blkno, 1, file_blk) != SUCCESS) {
                return -EIO;
            }
        }
    }

    // clear block between old_size and offset if extending file
    if (old_size < offset) {
        memset(file_blk + old_size, 0, offset - old_size);
    }

    // write file block contents to disk
    memcpy((uint8_t*)file_blk + offset, content, n_bytes);
    if (fs->dev->ops->write(fs->dev, file_blkno, 1, file_blk) != SUCCESS) {
        return -EIO;
    }

    // update file inode for file block
    fs->inodes[file_ino].mtime = time(NULL); // update modify time
    fs->inodes[file_ino].size = (new_size > old_size) ? new_size : old_size; // update size
    fs_mark_inode(fs, file_ino);  // mark inode changed

    fs_sync_metadata(fs);  // sync changed metadata
    return 0;  // success
}

/**
 * Write contents to a file.
 *
 * Note: current implementation supports
 * only 1 block of content
 *
 * Errors
 *   -ENISDIR  - dir_ino not a directory
 *   -ENOSPC   - free entry or block not found
 *   -EFBIG    - content too large
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param file_ino inode of file
 * @param content the content
 * @param n_bytes number of bytes
 * @return 0 if successful, -error if error occurred
 */
int fs_writefile(struct fs_ext2 *fs, int file_ino, const void *content, int n_bytes)
{
    // internal INT_MIN offset for truncation
    return fs_pwritefile(fs, file_ino, content, n_bytes, INT_MIN);
}