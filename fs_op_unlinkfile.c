/*
 * file: fs_op_unlinkfile.c
 *
 * description: unlink a file or empty subdirectory
 * in a directory for CS 5600 / 7600 file system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Philip Gust, March 2021
 */

#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#include "fs_op_unlinkfile.h"
#include "fs_dev_blkdev.h"

/**
 * Look up a directory entry in a directory block.
 *
 * Errors
 *   -ENOENT   - does not exists
 *
 * @param de storage for a directory block
 * @param name the entry name
 * @param ignore_case 1 for case-independent, 0 for case-dependent
 * @return entry in directory block or -error if not available
 */
static int get_entry_in_block(struct fs_dirent* de, const char *name, int ignore_case) {
    // case-dependent or case-dependent comparison
    static int (*compare[2])(const char*, const char*) = {strcmp, strcasecmp};

    // find free directory entry
    for (int i = 0; i < DIRENTS_PER_BLK; i++) {
        if (de[i].valid == 1) {
            // check for duplicate existing entry
            if (compare[ignore_case == 1](de[i].name, name) == 0) {
                return i;
            }
        }
    }
    return -ENOENT;
}

/**
 * Determines whether inode directory is empty.
 * Directory is empty if it only has 2 entries,
 * ("." and "..").
 *
 * Errors
 *   -ENOTDIR   - inode not s directory
 *
 * @param fs the file system
 * @param ino the inode
 * @return 1 if empty, 0 if not empty, or -error if not a directory
 */
static int is_dir_empty(struct fs_ext2 *fs, int ino) {
    if (!S_ISDIR(fs->inodes[ino].mode)) {
        return -ENOTDIR;
    }
    // empty if only ".' and '..' entries present
    return (fs->inodes[ino].size == 2*sizeof(struct fs_dirent));
}

/**
 * Return a inode to the free list.
 *
 * @param  inum the inode number
 */
static void return_inode(struct fs_ext2 *fs, int ino)
{
    // mark inode free
    FD_CLR(ino, fs->inode_map);
    fs_mark_inode(fs, ino); // inode metadata changed
}

/**
 * Return a block to the free list.
 *
 * @param  blkno the block number
 */
static void return_blk(struct fs_ext2 *fs, int blkno)
{
    // mark inode free
    FD_CLR(blkno, fs->block_map);
    fs_mark_blk(fs, blkno); // block metadata changed
}

/**
 * Unlink file or empty subdirectory if it matches the
 * specified type mask.
 * <p>
 * The type mask combines file type masks for permitted
 * file types. Use S_IFMT for any file type, S_IFDIR for
 * directory only, S_IFREG for a regular file only, or
 * (S_IFMT & ~S_IFDIR) for any type except a directory.
 * <p>
 * The subject file type is masked with this type mask
 * to determine whether it can be unlinked. The value
 * -EINVAL is returned if the file type does not match.
 *
 * Errors
 *   -ENOTDIR  - dir_ino not a directory
 *   -ENOTEMPTY - file_ino subdirectory not empty
 *   -ENOENT   - file_ino not child of dir_ino
 *   -EPERM    - not allowed
 *   -EINVAL   - wrong file type
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param dir_ino inode of parent directory
 * @param name the name of the file in the directory
 * @param typemask the type mask to determine eligibility
 * @return 0 if successful, -error if cannot create
 */
static int do_unlink(struct fs_ext2 *fs, int dir_ino, const char *name, int typemask)
{
    // ensure dir_ino is a directory
    if (!S_ISDIR(fs->inodes[dir_ino].mode)) {
        return -ENOTDIR;
    }

    // get block number of first directory block
    int dir_blkno = fs->inodes[dir_ino].direct[0];
    if (dir_blkno == 0) {
        return -ENOENT;  // no entry for file_ino
    }

    // read directory block
    struct fs_dirent de[DIRENTS_PER_BLK];
    if (fs->dev->ops->read(fs->dev, dir_blkno, 1, de) != SUCCESS) {
        return -EIO;  // cannot read block
    }

    // find directory entry for file inode
    int entry = get_entry_in_block(de, name, fs->ignore_case);
    if (entry < 0) {
        return entry;
    }

    // get inode for file
    int file_ino = de[entry].inode;

    // does match required file type
    if ((fs->inodes[file_ino].mode & typemask) == 0) {
        return -EINVAL;
    }

    // cannot delete "." or ".." entries
    if (   (strcmp(name, ".") == 0)
        || (strcmp(name, "..") == 0)) {
        return -EPERM;  // not allowed
    }

    // determines whether file_ino is an empty subdirectory
    int empty_subdir = is_dir_empty(fs, file_ino);

    // ensure directory is empty (only '.' and '..' entries)
    if (empty_subdir == 0) {
        return -ENOTEMPTY;
    }

    // remove the parent directory entry
    de[entry].valid = 0;

    // write parent directory block back to disk
    if (fs->dev->ops->write(fs->dev, dir_blkno, 1, de) != SUCCESS) {
        return -EIO;  // cannot write block
    }
    fs_mark_blk(fs, dir_blkno);  // block metadata changed

    // decrement parent directory size by size of directory entry
    fs->inodes[dir_ino].size -= sizeof(struct fs_dirent);
    // update modified time
    int t = time(NULL);
    fs->inodes[dir_ino].mtime = t;
    fs_mark_inode(fs, dir_ino);  // dir inode metadata changed

    if (empty_subdir == 1) { // file is an empty subdir
        // decrement parent directory link count for ".."
        fs->inodes[dir_ino].nlink--;
        // decrement child directory link count for "."
        fs->inodes[file_ino].nlink--;
    }

    // decrement child link count for removal from parent
    fs->inodes[file_ino].nlink--;
    fs_mark_inode(fs, file_ino);

    // if child link count now 0, free inode and block
    if (fs->inodes[file_ino].nlink == 0) {
        // free the inode block
        int file_blkno = fs->inodes[file_ino].direct[0];
        return_blk(fs, file_blkno);
        fs->inodes[file_ino].direct[0] = 0; // clear block number

        // free the inode
        return_inode(fs, file_ino);
    }

    fs_sync_metadata(fs);  // sync changed metadata
    return 0;
}

/**
 * Unlink file from a directory.
 *
 * Errors
 *   -EISDIR   - file_ino is a directory
 *   -ENOTDIR  - dir_ino not a directory
 *   -ENOENT   - file_ino not child of dir_ino
 *   -EPERM    - not allowed
 *   -EINVAL   - wrong file type
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param dir_ino inode of parent directory
 * @param name name of the file in the directory
 * @return 0 if successful, -error if cannot create
 */
int fs_unlinkfile(struct fs_ext2 *fs, int dir_ino, const char *name) {
    static int not_dir_mask = (S_IFMT & ~S_IFDIR);  // any but dir
    int result = do_unlink(fs, dir_ino, name, not_dir_mask);
    return (result == -EINVAL) ? -EISDIR : result;
}

/**
 * Remove subdirectory from a directory.
 *
 * Errors
 *   -ENOTDIR  - dir_ino not a directory
 *   -ENOTDIR  - file_ino not a directory
 *   -ENOTEMPTY - subdirectory not empty
 *   -EPERM    - not allowed
 *   -ENOENT   - file_ino not child of dir_ino
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param dir_ino inode of parent directory
 * @param name name of the subdirectory
 * @return 0 if successful, -error if cannot create
 */
int fs_rmdir(struct fs_ext2 *fs, int dir_ino, const char *name) {
    int result = do_unlink(fs, dir_ino, name, S_IFDIR); // dir only
    return (result == -EINVAL) ? -ENOTDIR : result;

}

/**
 * Unlink file or empty subdirectory from a directory.
 *
 * Errors
 *   -ENOTDIR  - dir_ino not a directory
 *   -ENOTEMPTY - file_ino subdirectory not empty
 *   -EPERM    - not allowed
 *   -ENOENT   - file_ino not child of dir_ino
 *   -EIO      - i/o error
 *
 * @param fs the file system
 * @param dir_ino inode of parent directory
 * @param name name of the file or subdirectory in the directory
 * @return 0 if successful, -error if cannot create
 */
int fs_unlinkat(struct fs_ext2 *fs, int dir_ino, const char *name) {
    return do_unlink(fs, dir_ino, name, S_IFMT); // any type
}