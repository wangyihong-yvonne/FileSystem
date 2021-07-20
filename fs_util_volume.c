/*
 * fs_util_volume.c
 *
 * description: manage a file system volume
 * for CS 7600 / CS 5600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Peter Desnoyers,  November 2016
 * Philip Gust, March 2021
 */

#include <stdlib.h>
#include <string.h>
#include "fs_dev_blkdev.h"
#include "fs_util_volume.h"
#include "fsx600.h"

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
 * Mount a ext2 file system volume on a block device.
 *
 * @param disk the block device
 * @param the file system volume
 * @return file system volume or NULL if cannot
 */
struct fs_ext2 *fs_mount_volume(struct fs_dev_blkdev* dev)
{
    block *meta = NULL;
    struct fs_ext2 *fs = malloc(sizeof (struct fs_ext2));
    if (fs == NULL) {
        goto err;
    }
    // initialize file system params
    fs->dev = dev;
    fs->n_blocks = dev->ops->num_blocks(dev);

    // read the superblock
    struct fs_super sb;
    if (dev->ops->read(dev, 0, 1, &sb) < 0) {
        goto err;
    }

    // not valid FS
    if (sb.magic != FS_MAGIC) {
        goto err;
    }

    // read volume metadata
    fs->n_meta = 1 + sb.inode_map_sz + sb.block_map_sz + sb.inode_region_sz;
    meta = malloc(fs->n_meta*FS_BLOCK_SIZE);
    if (meta == NULL) {
        goto err;
    }
    if (dev->ops->read(dev, 0, fs->n_meta, meta) < 0) {
        goto err;
    }
    fs->meta = meta;

    // record root inode
    fs->root_inode = sb.root_inode;

    // set case flags for file names
    fs->ignore_case = sb.ignore_case;
    fs->fold_case = sb.fold_case;

    // read inode map
    fs->inode_map_base = 1;
    fs->inode_map = (void*)&meta[fs->inode_map_base];

    // set block map
    fs->block_map_base = fs->inode_map_base + sb.inode_map_sz;
    fs->block_map = (void*)&meta[fs->block_map_base];

    // set inodes
    fs->inode_base = fs->block_map_base + sb.block_map_sz;
    fs->inodes = (void*)&meta[fs->inode_base];

    // set number of inodes
    fs->n_inodes = sb.inode_region_sz * INODES_PER_BLK;


    // set metadata map to mark modified metadata blocks
    const int n_meta_map = div_round_up(fs->n_meta, 8);
    fs->meta_map = malloc(n_meta_map);
    memset(fs->meta_map, 0, n_meta_map);

    // return mounted fs volume
    return fs;

    err:  // cleanup if error
    free(fs);
    free(meta);
    return NULL;
}

/**
 * Mark inode metadata changed.
 *
 * @param fs the file system
 * @param ino the inode
 */
void fs_mark_inode(struct fs_ext2 *fs, int ino) {
    // mark inode map block changed
    int inode_map_blk = fs->inode_map_base + ino/BITS_PER_BLK;
    FD_SET(inode_map_blk, fs->meta_map);

    // mark inode block changed
    int inode_blk = fs->inode_base + ino/INODES_PER_BLK;
    FD_SET(inode_blk, fs->meta_map);
}

/**
 * Mark block metadata changed.
 *
 * @param fs the file system
 * @param ino the inode
 */
void fs_mark_blk(struct fs_ext2 *fs, int blk) {
    int blk_map_blk = fs->block_map_base + blk/BITS_PER_BLK;
    FD_SET(blk_map_blk, fs->meta_map);
}

/**
 * Synchronize changed file system volume metadata
 * blocks to disk.
 *
 * @param fs the file system
 */
void fs_sync_metadata(struct fs_ext2 *fs) {
    // write changed metadata blocks to disk
    for (int i = 0; i < fs->n_meta; i++) {
        if (FD_ISSET(i, fs->meta_map)) {
            fs->dev->ops->write(fs->dev, i, 1, fs->meta[i]);
            FD_CLR(i, fs->meta_map);
        }
    }
}

/**
 * Synchronize file system volume to disk.
 *
 * @param fs the file system
 */
void fs_sync_volume(struct fs_ext2 *fs) {
    // flush metadata blocks to disk
    fs_sync_metadata(fs);

    // flush device volume blocks
    fs->dev->ops->flush(fs->dev, 0, fs->n_blocks);
}

/**
 * Unmounts the file system volume. Does not close
 * disk device.
 *
 * @param fs the file system
 * @return the underlying block device
 */
struct fs_dev_blkdev *fs_unmount_volume(struct fs_ext2 *fs)
{
    struct fs_dev_blkdev *dev = fs->dev;

    // flush metadata to disk
    fs_sync_volume(fs);

    // free metadata
    free(fs->meta);
    free(fs->meta_map);
    memset(fs, 0, sizeof(struct fs_ext2)); // kill fs struct
    free(fs);

    return dev;
}