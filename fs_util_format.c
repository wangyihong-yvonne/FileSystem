/*
 * fs_util_format.c
 *
 * description: format a volume file system
 * for CS 7600 / CS 5600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Peter Desnoyers,  November 2016
 * Philip Gust, March 2021
 */

#include <sys/select.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "fs_dev_blkdev.h"
#include "fs_util_format.h"
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
 * Format a file system volume for block device.
 * New volume occupies entire block device.
 *
 * Errors
 *   -EIO      - i/o error
 *
 * @param dev the block device
 * @param ignore_case ignore case for directory entries
 * @param fold_case fold case for directory entries
 * @return status 0 for success, -error for errors
 */
int fs_format_volume(struct fs_dev_blkdev* dev, int ignore_case, int fold_case)
{
    // number of blocks for device
    const int n_blks = dev->ops->num_blocks(dev);

    // calculate number of blocks in metadata segments
    const int n_inos = div_round_up(n_blks, 4);  // 1 inode for every 4 blocks
    const int n_ino_map_blks = div_round_up(n_inos, BITS_PER_BLK);
    const int n_ino_blks = div_round_up(n_inos*sizeof(struct fs_inode), FS_BLOCK_SIZE);
    const int n_map_blks = div_round_up(n_blks, BITS_PER_BLK);
    const int n_meta_blks = 1 + n_ino_map_blks + n_map_blks + n_ino_blks;
    const int root_ino = 1;

    // initialize file system metadata blocks
    block meta[n_meta_blks];
    memset(meta, 0, n_meta_blks * FS_BLOCK_SIZE);

    // initialize superblock
    const int superblk = 0;
    struct fs_super* sb = (void*)&meta[0];
    *sb = (struct fs_super) {
            .magic = FS_MAGIC,
            .inode_map_sz = n_ino_map_blks,
            .inode_region_sz = n_ino_blks,
            .block_map_sz = n_map_blks,
            .num_blocks = n_blks,
            .fold_case = fold_case,
            .ignore_case = (ignore_case || fold_case), // ignore case if folding
            .root_inode = root_ino
    };

    // initialize inode bitmap
    const int inode_map_base = 1;
    fd_set* inode_map = (void*)&meta[inode_map_base];

    // set root directory inode map allocated
    FD_SET(0, inode_map);  // inode 0 is unused
    FD_SET(root_ino, inode_map);  // inode for root directory

    // initialize block bitmap
    const int block_map_base = inode_map_base + n_ino_map_blks;
    fd_set* block_map = (void*)&meta[block_map_base];

    // initialize root directory inode
    const int inode_base = block_map_base + n_map_blks;
    struct fs_inode* inodes = (void*)&meta[inode_base];

    int t  = time(NULL);
    const int rootdir_blkno = n_meta_blks; // after metadata blocks
    inodes[root_ino] = (struct fs_inode) {
        .uid = 1001, .gid = 125, // user group id
        .mode = (S_IFDIR | 0755), // directory w/ permissions rwxr-xr-x
        .ctime = t, .mtime = t,
        .size = 0, .nlink = 0,
        .direct = {rootdir_blkno, 0, 0, 0, 0, 0},
        .indir_1 = 0, .indir_2 = 0
    };

    // set all metadata blocks allocated
    for (int i = 0; i < n_meta_blks; i++) {
        FD_SET(i, block_map);
    }

    // set root dir block allocated
    FD_SET(rootdir_blkno, block_map);

    // init root directory block for "." and ".." entries
    struct fs_dirent root_de[DIRENTS_PER_BLK];
    memset(root_de, 0, FS_BLOCK_SIZE);

    //  entry "/.", links to "/" directory
    root_de[0] = (struct fs_dirent) {
        .valid = 1, .isDir = 1,
        .inode = root_ino,  // links to self
        .name = "."
    };

    //  entry "/..", links to "/" directory (special for root "..")
    root_de[1] = (struct fs_dirent) {
        .valid = 1, .isDir = 1,
        .inode = root_ino,  // parent links to self
        .name = ".."
    };

    // write root dir block to block device
    if (dev->ops->write(dev, rootdir_blkno, 1, root_de) != SUCCESS) {
        return -EIO;
    }

    // update root dir inode for "." and ".." entries
    inodes[root_ino].size += 2*sizeof(struct fs_dirent);
    inodes[root_ino].nlink+= 2;	// links for "." and ".."

    // write file system metadata to block device
    if (dev->ops->write(dev, 0, n_meta_blks, meta) != SUCCESS) {
        return -EIO;
    }

    return 0;  // successful
}