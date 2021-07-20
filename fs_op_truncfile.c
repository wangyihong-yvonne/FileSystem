/**
 *
 * fs_op_truncfile.c
 *
 * description: truncate file for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#include "fs_op_writefile.h"
#include "fs_dev_blkdev.h"

/**
 * Cause the file to be truncated (or extended)
 * to n_bytes bytes in size. If the file size
 * exceeds length, any extra data is discarded.
 * If the file size is smaller than length, the
 * file is extended and filled with zeros to the
 * indicated length.
 *
 * Note: current implementation supports
 * only 1 block of content
 *
 * Errors
 *   -EISDIR   - file_in is a directory
 *   -ENOSPC   - free entry or block not found
 *   -EFBIG    - content too large
 *   -EACCES   - if no write permission
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param file_ino inode of file
 * @param content the content
 * @param n_bytes number of bytes
 * @return 0 if successful, -error if error occurred
 */
int fs_truncfile(struct fs_ext2 *fs, int file_ino, int n_bytes)
{
    // ensure file_ino is a regular file
    if (!S_ISREG(fs->inodes[file_ino].mode)) {
        return -EISDIR;
    }

    // ensure bytes fit in block
    if ((n_bytes < 0) || (n_bytes > FS_BLOCK_SIZE)) {
        return -EFBIG;  // contents too large/small
    }

    // get block number of first file block
    int file_blkno = fs->inodes[file_ino].direct[0];
    if (file_blkno == 0) {
        return -ENOSPC;  // no file block
    }

    // no change
    int cur_bytes = fs->inodes[file_ino].size;
    if (n_bytes == cur_bytes) {
        return 0;
    }

    // update block to zero extended bytes
    if (n_bytes > cur_bytes) {
        // read file block from disk
        block file_blk;
        if (fs->dev->ops->read(fs->dev, file_blkno, 1, file_blk) != SUCCESS) {
            return -EIO;
        }
        memset(((char*)file_blk)+cur_bytes, 0, n_bytes-cur_bytes);

        // write block back to disk
        if (fs->dev->ops->write(fs->dev, file_blkno, 1, file_blk) != SUCCESS) {
            return -EIO;
        }
    }

    // update file inode for file block
    fs->inodes[file_ino].mtime = time(NULL); // update modify time
    fs->inodes[file_ino].size = n_bytes; // update size
    fs_mark_inode(fs, file_ino);  // mark dir inode changed

    fs_sync_metadata(fs);  // sync changed metadata
    return 0;  // success
}
