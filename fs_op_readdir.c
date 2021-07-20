/*
 * fs_op_readdir.c
 *
 * description: read directory for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#include <stdlib.h>
#include <sys/stat.h>

#include "fs_op_readdir.h"
#include "fs_dev_blkdev.h"
#include "fsx600.h"

/** struct for a directory stream */
struct FS_DIR {
    struct fs_dirent *de;
    int cur_entry;
};

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
FS_DIR* fs_opendir(struct fs_ext2 *fs, int dir_ino) {
    // ensure dir_ino is a directory
    if (!S_ISDIR(fs->inodes[dir_ino].mode)) {
        return NULL;
    }

    // create and initialize directory stream
    FS_DIR *dir = malloc(sizeof(FS_DIR));
    dir->de = calloc(1, FS_BLOCK_SIZE);
    dir->cur_entry = 0;

    // get block number of first file block
    int file_blkno = fs->inodes[dir_ino].direct[0];
    if (file_blkno > 0) {
        // read file block from disk
        if (fs->dev->ops->read(fs->dev, file_blkno, 1, dir->de) == SUCCESS) {
            return dir;
        }
    }

    // cannot read directory
    free(dir->de);
    free(dir);
    return NULL;
}

/**
 * Closes the directory stream associated with dirp.
 *
 * @param dirp the directory stream
 */
void fs_closedir(FS_DIR *dirp) {
    free(dirp->de);
    free(dirp);
}

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
struct fs_dirent *fs_readdir(FS_DIR *dirp)
{
    for ( ; dirp->cur_entry < DIRENTS_PER_BLK; dirp->cur_entry++) {
        // find and return next valid entry
        if (dirp->de[dirp->cur_entry].valid) {
            return &dirp->de[dirp->cur_entry++];  // move cursor for next read
        }
    }
    // no more entries
    return NULL;
}