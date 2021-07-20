/*
 * fs_util_volume.h
 *
 * description: manage a file system volume
 * for CS 7600 / CS 5600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Peter Desnoyers,  November 2016
 * Philip Gust, March 2021
 */

#ifndef FS_UTIL_VOLUME_H_
#define FS_UTIL_VOLUME_H_

#include <sys/select.h>
#include "fsx600.h"
#include "fs_dev_blkdev.h"

/** information about ext2 fs volume */
struct fs_ext2 {
    /** disk device */
    struct fs_dev_blkdev* dev;

    /** number of available blocks */
    int n_blocks;

    /** number of metadata blocks */
    int n_meta;

    /** volume metadata */
    block *meta;

    /** blkno of first inode map block */
    int inode_map_base;

    /** pointer to inode bitmap */
    fd_set *inode_map;

    /** number of inodes from superblock */
    int n_inodes;

    /** blkno of first inode block */
    int inode_base;

    /** pointer to inode blocks */
    struct fs_inode *inodes;

    /** number of root inode from superblock */
    int root_inode;

    /** blkno of first data block */
    int block_map_base;

    /** pointer to block bitmap to determine free blocks */
    fd_set *block_map;

    /** pointer to metadata bitmap */
    fd_set *meta_map;

    // fold case when storing names (1=fold, 0=preserve)
    int fold_case;

    // ignore case when comparing names (1=ci comparison, 1=exact comparison
    int ignore_case;
};

/**
 * Mount a file system volume on a block device.
 *
 * @param disk the block device
 * @param the file system volume
 */
struct fs_ext2 *fs_mount_volume(struct fs_dev_blkdev* dev);

/**
 * Synchronize file system volume metadata to disk.
 *
 * @param fs the file system
 */
void fs_sync_volume(struct fs_ext2 *fs);

/**
 * Unmounts the file system volume. Does not close
 * disk device.
 *
 * @param fs the file system
 * @return the underlying block device
 */
struct fs_dev_blkdev *fs_unmount_volume(struct fs_ext2 *fs);

/**
 * Mark inode metadata changed.
 *
 * @param fs the file system
 * @param ino the inode
 */
void fs_mark_inode(struct fs_ext2 *fs, int ino);

/**
 * Mark block metadata changed.
 *
 * @param fs the file system
 * @param ino the inode
 */
void fs_mark_blk(struct fs_ext2 *fs, int blk);

/**
 * Synchronize changed file system volume metadata
 * blocks to disk.
 *
 * @param fs the file system
 */
void fs_sync_metadata(struct fs_ext2 *fs);

/**
 * Synchronize file system volume to disk.
 *
 * @param fs the file system
 */
void fs_sync_volume(struct fs_ext2 *fs);

#endif /* FS_UTIL_VOLUME_H_ */
