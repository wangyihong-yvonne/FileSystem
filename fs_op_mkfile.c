/*
 * fs_op_makefile.c
 *
 * description: make a file, dir, or link in a directory
 * for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>

#include "fs_op_mkfile.h"
#include "fs_dev_blkdev.h"

/**
 * Gets a free inode number from the free list.
 *
 * Errors
 *   -ENOSPC   - free entry not found
 *
 * @param fs the file system
 * @return a free inode number or -error if none available
 */
static int get_free_inode(struct fs_ext2 *fs)
{
    // allocate inode for file
    for (int i = 1; i < fs->n_inodes; i++) {
        if (FD_ISSET(i, fs->inode_map) == 0) {
            FD_SET(i, fs->inode_map);  // mark allocated
            fs_mark_inode(fs, i);  // mark inode metadata changed
            return i;
        }
    }
    return -ENOSPC;
}

/**
 * Gets a free block number from the free list.
 *
 * Errors
 *   -ENOSPC   - free entry not found
 *
 * @param fs the file system
 * @return free block number or -error if none available
 */
static int get_free_blk(struct fs_ext2 *fs)
{
    for (int i = fs->n_meta; i < fs->n_blocks; i++) {
        if (FD_ISSET(i, fs->block_map) == 0) {
            FD_SET(i, fs->block_map);  // mark allocated
            fs_mark_blk(fs, i);  // mark blk metadata changed
            return i;
        }
    }
    return -ENOSPC;
}

/**
 * Fold case of a character string in place.
 *
 * @param str the string
 */
static void fold_case(char *str)
{
    for (char *p = str; *p != '\0'; p++) {
        *p = (char)toupper(*p);
    }
}

/**
 * Find free entry in a directory block for
 * named file.
 *
 * Errors
 *   -EEXIST   - entry already exists
 *   -ENOSPC   - free entry not found
 *
 * @param de storage for a directory block
 * @param name the name of the entry
 * @param ignore_case 1 for case-independent, 0 for case-dependent
 * @return entry in directory block or -error if
 *  dup or not available
 */
static int get_free_entry_in_block(struct fs_dirent* de, const char *name, int ignore_case)
{
    // case-dependent or case-dependent comparison
    static int (*compare[2])(const char*, const char*) = {strcmp, strcasecmp};

    // find free directory entry;
    // scan entire directory to check for dups
    int entry = -ENOSPC;
    for (int i = 0; i < DIRENTS_PER_BLK; i++) {
        if (de[i].valid == 1) {
            // check for duplicate existing entry
            if (compare[ignore_case==1](de[i].name, name) == 0) {
                return -EEXIST;
            }
        } else if (entry == -ENOSPC) { // first found
            entry = i;
        }
    }
    return entry;
}

/**
 * Make file, directory, or link to existing
 * inode in a directory.
 *
 * Errors
 *   -ENAMETOOLONG  - name too long
 *   -ENOTDIR  - dir_ino not a directory
 *   -EISDIR  - link file inode is directory
 *   -EEXIST   - entry already exists
 *   -ENOSPC   - free entry or block not found
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param dir_ino inode of parent directory
 * @param name the file name
 * @param mode creation mode
 * @param flag -S_IFDIR for new directory,
 *  -S_IFREG for new file, >0 for existing file inode
 * @return inode of file if successful, -error if cannot create
 */
static int mkentry(struct fs_ext2 *fs, int dir_ino, const char* name, mode_t mode, int flag)
{
    // ensure the file name is not too long
    if (strlen(name) >= FS_FILENAME_SIZE) {
        return -ENAMETOOLONG; // name too long
    }

    // ensure dir_ino is a directory
    if (!S_ISDIR(fs->inodes[dir_ino].mode)) {
        return -ENOTDIR;
    }

    // get block number of first directory block
    int dir_blkno = fs->inodes[dir_ino].direct[0];
    if (dir_blkno == 0) {
        return -ENOSPC;  // no directory block
    }

    // read the directory block
    struct fs_dirent dir_de[DIRENTS_PER_BLK];
    if (fs->dev->ops->read(fs->dev, dir_blkno, 1, dir_de) != SUCCESS) {
        return -EIO;  // cannot read block
    }

    // find free directory entry for name
    int entry = get_free_entry_in_block(dir_de, name, fs->ignore_case);
    if (entry < 0) {
        return entry;
    }

    int file_ino, file_blkno;
    time_t t = time(NULL);

    if (flag > 0) {  // link to existing inode
        file_ino = flag;

        // cannot link a directory
        if (S_ISDIR(fs->inodes[file_ino].mode)) {
            return -EISDIR;
        }
    } else {  // create a new file or subdir
        // allocate block for new file
        file_blkno = get_free_blk(fs);
        if (file_blkno < 0) {
            return file_blkno;
        }

        // get free inode for new file
        file_ino = get_free_inode(fs);
        if (file_ino < 0) {
            return file_ino;  // no inode available
        }

        // initialize file inode for new file
        int type = (-flag) & S_IFMT;  // isolate file type
        int perm = mode & 0777;  // isolate file permissions
        fs->inodes[file_ino] = (struct fs_inode) {
                .uid = 1001, // inode owner
                .gid = fs->inodes[dir_ino].gid, // inode group matches directory
                .mode = (type | perm), // combine type and permissions
                .ctime = t, .mtime = t,
                .size = 0, .nlink = 0,
                .direct = {file_blkno, 0, 0, 0, 0, 0},
                .indir_1 = 0, .indir_2 = 0
        };
    }

    // add file inode for new file to directory
    dir_de[entry] = (struct fs_dirent) {
            .valid = 1,			// entry valid
            .isDir = (flag == -S_IFDIR),	// 1 if directory or 0 otherwise
            .inode = file_ino	// file inode
    };
    strcpy(dir_de[entry].name, name); // set file name
    if (fs->fold_case) {  // fold if not case preserving
        fold_case(dir_de[entry].name);
    }

    // write directory block with new file entry back to disk
    if (fs->dev->ops->write(fs->dev, dir_blkno, 1, dir_de) != SUCCESS) {
        return -EIO;
    }

    // increase size of dir_inode by size of new directory entry
    fs->inodes[dir_ino].size += sizeof(struct fs_dirent);
    fs->inodes[dir_ino].mtime = t;
    fs_mark_inode(fs, dir_ino);  // mark dir inode changed

    // increment link count of file for new directory entry
    fs->inodes[file_ino].nlink++;
    fs_mark_inode(fs, file_ino);

    // make "." and ".." entries in new subdirectory block
    if (flag == -S_IFDIR) {
        // init directory block for "." and ".." entries
        struct fs_dirent subdir_de[DIRENTS_PER_BLK];
        memset(subdir_de, 0, FS_BLOCK_SIZE);

        // entry ".", links to new subdirectory
        subdir_de[0] = (struct fs_dirent) {
                .valid = 1, .isDir = 1,
                .inode = file_ino,  // points to self
                .name = "."
        };

        // entry "..", links to parent directory
        subdir_de[1] = (struct fs_dirent) {
                .valid = 1, .isDir = 1,
                .inode = dir_ino,  // points to parent
                .name = ".."
        };

        // write subdirectory block to disk
        if (fs->dev->ops->write(fs->dev, file_blkno, 1, subdir_de) != SUCCESS) {
            return -EIO;
        }

        // increase size of subdir inode for "." and ".." entries
        fs->inodes[file_ino].size += 2*sizeof(struct fs_dirent);

        // increase link count of subdir inode for "." subdir entry
        fs->inodes[file_ino].nlink++;

        // increase link count of dir inode for ".." subdir entry
        fs->inodes[dir_ino].nlink++;
    }

    fs_sync_metadata(fs);  // sync changed metadata
    return file_ino;  // success
}

/**
 * Make file in a directory.
 *
 * Errors
 *   -ENAMETOOLONG  - name too long
 *   -ENOTDIR  - dir_ino not a directory
 *   -EEXIST   - entry already exists
 *   -ENOSPC   - free entry or block not found
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param dir_ino inode of parent directory
 * @param name the file name
 * @param mode creation mode
 * @return inode of file if successful, -error if cannot create
 */
int fs_mkfile(struct fs_ext2 *fs, int dir_ino, const char* name, mode_t mode) {
    return mkentry(fs, dir_ino, name, mode, -S_IFREG);
}

/**
 * Make subdirectory in a directory.
 *
 * Errors
 *   -ENAMETOOLONG  - name too long
 *   -ENOTDIR  - dir_ino not a directory
 *   -EEXIST   - entry already exists
 *   -ENOSPC   - free entry or block not found
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param dir_ino inode of parent directory
 * @param name the subdirectory name
 * @param mode creation mode
 * @return inode of file if successful, -error if cannot create
 */
int fs_mkdir(struct fs_ext2 *fs, int dir_ino, const char* name, mode_t mode) {
    return mkentry(fs, dir_ino, name, mode, -S_IFDIR);
}

/**
 * Make link to existing non-directory inode in a directory.
 *
 * Errors
 *   -ENAMETOOLONG  - name too long
 *   -ENOTDIR  - dir_ino not a directory
 *   -EISDIR   - link file inode is directory
 *   -EEXIST   - entry already exists
 *   -ENOSPC   - free entry or block not found
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param dir_ino inode of parent directory
 * @param file_ino inode of existing file inode
 * @param name the file name
 * @return inode of file if successful, -error if cannot link
 */
int fs_mklink(struct fs_ext2 *fs, int dir_ino, int file_ino, const char* name) {
    return mkentry(fs, dir_ino, name, 0, file_ino);
}

