/*
 * assignment-4-test.c
 *
 * description: test suite for CS 5600 / 7600 file system
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "fs_util_format.h"
#include "fs_util_volume.h"
#include "fs_util_verify.h"
#include "fs_op_mkfile.h"
#include "fs_op_unlinkfile.h"
#include "fs_op_readdir.h"
#include "fs_op_readfile.h"
#include "fs_op_writefile.h"
#include "fs_op_truncfile.h"
#include "fs_op_statfile.h"
#include "fs_op_statfs.h"
#include "fs_dev_memorydev.h"

/** Optional functions **/
#include "fs_op_chmodfile.h"
#include "fs_op_chownfile.h"
#include "fs_op_utimefile.h"

#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"

/**
 * Test memory device operations
 */
void test_mem_device(void) {
    const int n_blks = 4;

    // create memory block device
    struct fs_dev_blkdev *dev = memory_blkdev_create(n_blks);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev);

    // validate fields
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev->private);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev->ops);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev->ops->num_blocks);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev->ops->read);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev->ops->write);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev->ops->close);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev->ops->flush);

    int status;
    block writeblk[2];
    block readblk[2];
    strcpy((void*)&writeblk[0], "block 0");
    strcpy((void*)&writeblk[1], "block 1");

    // write 1 block to block 0
    CU_ASSERT_EQUAL(dev->ops->num_blocks(dev), n_blks);
    status = dev->ops->write(dev, 0, 1, &writeblk[0]);
    CU_ASSERT_EQUAL(status, SUCCESS);

    // read 1 block to block 0
    status = dev->ops->read(dev, 0, 1, &readblk[0]);
    CU_ASSERT_EQUAL(status, SUCCESS);
    status = memcmp(&writeblk[0], &readblk[0], FS_BLOCK_SIZE);
    CU_ASSERT_EQUAL(status, 0);

    // write 2 blocks to block 2 and 3
    CU_ASSERT_EQUAL(dev->ops->num_blocks(dev), n_blks);
    status = dev->ops->write(dev, 2, 2, &writeblk[0]);
    CU_ASSERT_EQUAL(status, SUCCESS);

    // read 2 blocks from blocks 2 and 3
    status = dev->ops->read(dev, 2, 2, &readblk[0]);
    CU_ASSERT_EQUAL(status, SUCCESS);
    status = memcmp(&writeblk[0], &readblk[0], 2*FS_BLOCK_SIZE);
    CU_ASSERT_EQUAL(status, 0);

    // write 2 blocks to block 3 and 4 -- should fail
    CU_ASSERT_EQUAL(dev->ops->num_blocks(dev), n_blks);
    status = dev->ops->write(dev, 3, 2, &writeblk[0]);
    CU_ASSERT_NOT_EQUAL(status, SUCCESS);

    // read 2 blocks from blocks 3 and 4
    status = dev->ops->read(dev, 3, 2, &readblk[0]);
    CU_ASSERT_NOT_EQUAL(status, SUCCESS);

    // close device
    dev->ops->close(dev);
}

/**
 * Test file system volume operations.
 */
void test_volume(void) {
    const int n_blks = 100;

    // create memory block device
    struct fs_dev_blkdev *dev = memory_blkdev_create(n_blks);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev);

    // ensure cannot mount unformatted file system volume
    struct fs_ext2 *fs = fs_mount_volume(dev);
    CU_ASSERT_PTR_NULL_FATAL(fs);

    // ensure can mount formatted file system volume
    int fmtstatus = fs_format_volume(dev, 0, 0);
    CU_ASSERT_EQUAL_FATAL(fmtstatus, 0);
    fs = fs_mount_volume(dev);
    CU_ASSERT_PTR_NOT_NULL_FATAL(fs);

    // ensure volume device set
    CU_ASSERT_PTR_NOT_NULL(fs->dev);
    // ensure volume block count
    CU_ASSERT_EQUAL(fs->n_blocks, n_blks);

    // ensure volume metadata pointers set
    CU_ASSERT_PTR_NOT_NULL(fs->inode_map);
    CU_ASSERT_PTR_NOT_NULL(fs->block_map);
    CU_ASSERT_PTR_NOT_NULL(fs->inodes);

    // unmount file system volume
    fs_unmount_volume(fs);

    // close device
    dev->ops->close(dev);
}

/**
 * Test  case-preserving, case-dependent (cpcd)
 * file system file operations.
 */
void test_file_cpcd(void) {
    const int n_blks = 100;
    const mode_t file_mode = 0644;  // rw-r--r--

    // create memory block device
    struct fs_dev_blkdev *dev = memory_blkdev_create(n_blks);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev);

    // format volume
    int fmtstatus = fs_format_volume(dev, 0, 0);
    CU_ASSERT_EQUAL_FATAL(fmtstatus, 0);

    // mount formatted volume
    struct fs_ext2 *fs = fs_mount_volume(dev);
    CU_ASSERT_PTR_NOT_NULL_FATAL(fs);

    // create "file1"
    int file1_ino = fs_mkfile(fs, fs->root_inode, "file1", file_mode);
    CU_ASSERT_TRUE(file1_ino > 0);

    // verify "file1" is empty
    CU_ASSERT_EQUAL(fs->inodes[file1_ino].size, 0);

    // verify "file1" has link count 1
    CU_ASSERT_EQUAL(fs->inodes[file1_ino].nlink, 1);

    // create "File1"
    int File1_ino = fs_mkfile(fs, fs->root_inode, "File1", file_mode);
    CU_ASSERT_TRUE(File1_ino > 0);

    // verify "File1" is empty
    CU_ASSERT_EQUAL(fs->inodes[File1_ino].size, 0);

    // verify "File1" has link count 1
    CU_ASSERT_EQUAL(fs->inodes[File1_ino].nlink, 1);

    // verify cannot create duplicate "File1"
    int File1_dup_ino = fs_mkfile(fs, fs->root_inode, "File1", file_mode);
    CU_ASSERT_FALSE(File1_dup_ino > 0);

    // remove "file1"
    int file1_status = fs_unlinkfile(fs, fs->root_inode, "file1");
    CU_ASSERT_EQUAL(file1_status, 0);

    // ensure file1_ino not allocated
    int file1_ino_set = FD_ISSET(file1_ino, fs->inode_map);
    CU_ASSERT_EQUAL(file1_ino_set, 0);

    // verify cannot remove "file1" again
    int file1_dup_status = fs_unlinkfile(fs, fs->root_inode, "file1");
    CU_ASSERT_NOT_EQUAL(file1_dup_status, 0);

    // remove "File1"
    int File1_status = fs_unlinkat(fs, fs->root_inode, "File1");
    CU_ASSERT_EQUAL(File1_status, 0);

    // ensure File1_ino not allocated
    int File1_ino_set = FD_ISSET(File1_ino, fs->inode_map);
    CU_ASSERT_EQUAL(File1_ino_set, 0);

    // unmount file system volume and close device
    fs_unmount_volume(fs);
    dev->ops->close(dev);
}

/**
 * Test case-preserving, case-case-independent (cpci)
 * file system file operations.
 */
void test_file_cpci(void) {
    const int n_blks = 100;
    const mode_t file_mode = 0644;  // rw-r--r--

    // create memory block device
    struct fs_dev_blkdev *dev = memory_blkdev_create(n_blks);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev);

    // format volume case-independent, case-preserving
    int fmtstatus = fs_format_volume(dev, 1, 0);
    CU_ASSERT_EQUAL_FATAL(fmtstatus, 0);

    // mount formatted volume
    struct fs_ext2 *fs = fs_mount_volume(dev);
    CU_ASSERT_PTR_NOT_NULL_FATAL(fs);

    // create "file1"
    int file1_ino = fs_mkfile(fs, fs->root_inode, "file1", file_mode);
    CU_ASSERT_TRUE(file1_ino > 0);


    // create "File2"
    int file2_ino = fs_mkfile(fs, fs->root_inode, "File2", file_mode);
    CU_ASSERT_TRUE(file2_ino > 0);

    // verify cannot create duplicate "file2"
    int file2_dup_ino = fs_mkfile(fs, fs->root_inode, "file2", file_mode);
    CU_ASSERT_FALSE(file2_dup_ino > 0);

    // remove "File1"
    int file1_status = fs_unlinkfile(fs, fs->root_inode, "File1");
    CU_ASSERT_EQUAL(file1_status, 0);

    // verify cannot remove "file1" again
    int file1_dup_status = fs_unlinkfile(fs, fs->root_inode, "file1");
    CU_ASSERT_NOT_EQUAL(file1_dup_status, 0);

    // remove "FILE2"
    int file2_status = fs_unlinkat(fs, fs->root_inode, "FILE2");
    CU_ASSERT_EQUAL(file2_status, 0);

    // unmount file system volume and close device
    fs_unmount_volume(fs);
    dev->ops->close(dev);
}

/**
 * Test file system directory operations.
 */
void test_dir(void) {
    const int n_blks = 100;
    const mode_t file_mode = 0644;  // rw-r--r--
    const mode_t dir_mode = 0755;   // rwxr-xr-xx

    // create memory block device
    struct fs_dev_blkdev *dev = memory_blkdev_create(n_blks);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev);

    // format volume
    int fmtstatus = fs_format_volume(dev, 0, 0);
    CU_ASSERT_EQUAL_FATAL(fmtstatus, 0);

    // mount formatted volume
    struct fs_ext2 *fs = fs_mount_volume(dev);
    CU_ASSERT_PTR_NOT_NULL_FATAL(fs);


    /// Verify state of empty root directory

    // expect 2 entries for "." and ".."
    // (size of directory is size of used fs_dirents)
    int root_size = fs->inodes[fs->root_inode].size;
    CU_ASSERT_EQUAL(root_size, 2*sizeof(struct fs_dirent));

    // verify root inode has link count 2 for "." and ".."
    int root_nlink = fs->inodes[fs->root_inode].nlink;
    CU_ASSERT_EQUAL(root_nlink, 2);

    // ensure cannot remove "." from root dir
    int dot_status = fs_rmdir(fs, fs->root_inode, ".");
    CU_ASSERT_NOT_EQUAL(dot_status, 0);


    /// Verify state of root directory when creating
    /// and deleting a regular file

    // get root dir mod time for comparison
    int root_time = fs->inodes[fs->root_inode].mtime;
    sleep(1); // time passes

    // create "file1"
    int file1_ino = fs_mkfile(fs, fs->root_inode, "file1", file_mode);
    CU_ASSERT_TRUE(file1_ino > 0);

    // verify root dir mode time changed
    CU_ASSERT_NOT_EQUAL(fs->inodes[fs->root_inode].mtime, root_time);

    // expect 3 entries for ".", "..", and "file1"
    // (size of directory is size of used fs_dirents)
    root_size = fs->inodes[fs->root_inode].size;
    CU_ASSERT_EQUAL(root_size, 3*sizeof(struct fs_dirent));

    // verify root inode still has link count 2 for "." and ".."
    root_nlink = fs->inodes[fs->root_inode].nlink;
    CU_ASSERT_EQUAL(root_nlink, 2);

    // get root dir mod time for comparison
    root_time = fs->inodes[fs->root_inode].mtime;
    sleep(1); // time passes

    // remove "file1"
    int file1_status = fs_unlinkfile(fs, fs->root_inode, "file1");
    CU_ASSERT_EQUAL(file1_status, 0);

    // verify root dir mode time changed
    CU_ASSERT_NOT_EQUAL(fs->inodes[fs->root_inode].mtime, root_time);

    // ensure file1_ino not allocated
    int file1_ino_set = FD_ISSET(file1_ino, fs->inode_map);
    CU_ASSERT_EQUAL(file1_ino_set, 0);

    // expect 2 entries for "." and ".."
    root_size = fs->inodes[fs->root_inode].size;
    CU_ASSERT_EQUAL(root_size, 2*sizeof(struct fs_dirent));

    // verify root inode still has link count 2 for "." and ".."
    root_nlink = fs->inodes[fs->root_inode].nlink;
    CU_ASSERT_EQUAL(root_nlink, 2);

    /// Verify state of root directory and new
    /// subdirectory "dir1"

    // create "dir1"
    int dir1_ino = fs_mkdir(fs, fs->root_inode, "dir1", dir_mode);
    CU_ASSERT_TRUE(dir1_ino > 0);

    // expect 2 entries for "." and ".."
    int dir1_size = fs->inodes[dir1_ino].size;
    CU_ASSERT_EQUAL(dir1_size, 2*sizeof(struct fs_dirent));

    // verify dir1_ino has link count 2
    int dir1_nlink = fs->inodes[dir1_ino].nlink;
    CU_ASSERT_EQUAL(dir1_nlink, 2);

    // expect 3 root entries for ".", "..", and "dir1"
    root_size = fs->inodes[fs->root_inode].size;
    CU_ASSERT_EQUAL(root_size, 3*sizeof(struct fs_dirent));

    // verify root_ino has link count 3 (".", "..", and "dir1/.."
    CU_ASSERT_EQUAL(fs->inodes[fs->root_inode].nlink, 3);

    // ensure cannot create duplicate "dir1"
    int dir1_dup_ino = fs_mkdir(fs, fs->root_inode, "dir1", dir_mode);
    CU_ASSERT_FALSE(dir1_dup_ino > 0);


    /// Verify state of root directory when creating
    /// a subdirectory "dir2"

    // create "dir2"
    int dir2_ino = fs_mkdir(fs, fs->root_inode, "dir2", dir_mode);
    CU_ASSERT_TRUE(dir2_ino > 0);

    // expect 4 root entries for ".","..", "dir1", and "dir2"
    root_size = fs->inodes[fs->root_inode].size;
    CU_ASSERT_EQUAL(root_size, 4*sizeof(struct fs_dirent));

    // verify root_ino has link count 4 (".", "..", "dir1/..", and "dir2/.."
    CU_ASSERT_EQUAL(fs->inodes[fs->root_inode].nlink, 4);


    /// Verify state of subdirectory "dir2" when creating
    /// and removing a regular file "file1 in it

    // create "file1" in "dir2"
    int dir2_file1_ino = fs_mkfile(fs, dir2_ino, "file1", file_mode);
    CU_ASSERT_TRUE(dir2_file1_ino > 0);

    // ensure cannot remove non-empty dir2
    int dir2_status = fs_rmdir(fs, fs->root_inode, "dir2");
    CU_ASSERT_NOT_EQUAL(dir2_status, 0);

    // ensure dir2_ino still allocated
    int dir2_ino_set = FD_ISSET(dir2_ino, fs->inode_map);
    CU_ASSERT_NOT_EQUAL(dir2_ino_set, 0);

    // remove "file1" from dir2
    int dir2_file1_status = fs_unlinkfile(fs, dir2_ino, "file1");
    CU_ASSERT_EQUAL(file1_status, 0);

    // ensure dir2_file1_ino not allocated
    int dir2_file1_ino_set = FD_ISSET(dir2_file1_ino, fs->inode_map);
    CU_ASSERT_EQUAL(dir2_file1_ino_set, 0);

    // ensure can remove empty dir2
    dir2_status = fs_rmdir(fs, fs->root_inode, "dir2");
    CU_ASSERT_EQUAL(dir2_status, 0);

    // ensure dir2_file1_ino not allocated
    dir2_ino_set = FD_ISSET(dir2_ino, fs->inode_map);
    CU_ASSERT_EQUAL(dir2_ino_set, 0);

    /// Verify state of root directory when removing
    /// empty subdirectory "dir1"

    // ensure can remove empty dir1
    int dir1_status = fs_unlinkat(fs, fs->root_inode, "dir1");
    CU_ASSERT_EQUAL(dir1_status, 0);

    // ensure dir1_ino not allocated
    int dir1_ino_set = FD_ISSET(dir1_ino, fs->inode_map);
    CU_ASSERT_EQUAL(dir1_ino_set, 0);

    // expect 2 entries for "." and ".."
    root_size = fs->inodes[fs->root_inode].size;
    CU_ASSERT_EQUAL(root_size, 2*sizeof(struct fs_dirent));

    // verify root inode has link count 2 for "." and ".."
    root_nlink = fs->inodes[fs->root_inode].nlink;
    CU_ASSERT_EQUAL(root_nlink, 2);


    // unmount file system volume and close device
    fs_unmount_volume(fs);
    dev->ops->close(dev);
}

/**
 * Test file system link operators
 */
void test_link(void) {
    const int n_blks = 100;
    const mode_t file_mode = 0644;  // rw-r--r--
    const mode_t dir_mode = 0755;   // rwxr-xr-xx

    // create memory block device
    struct fs_dev_blkdev *dev = memory_blkdev_create(n_blks);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev);

    // format volume
    int fmtstatus = fs_format_volume(dev, 0, 0);
    CU_ASSERT_EQUAL_FATAL(fmtstatus, 0);

    // mount formatted volume
    struct fs_ext2 *fs = fs_mount_volume(dev);
    CU_ASSERT_PTR_NOT_NULL_FATAL(fs);

    /// ensure can create link for file inode in same directory

    // create "file1"
    int file1_ino = fs_mkfile(fs, fs->root_inode, "file1", file_mode);
    CU_ASSERT_TRUE(file1_ino > 0);

    // create duplicate "file1" link
    int file1_dup_ino = fs_mklink(fs, fs->root_inode, file1_ino, "file1");
    CU_ASSERT_FALSE(file1_dup_ino > 0);

    // create "file2" link
    int file2_ino = fs_mklink(fs, fs->root_inode, file1_ino, "file2");
    CU_ASSERT_TRUE(file2_ino > 0);

    // verify file1_ino matches file2_ino
    CU_ASSERT_EQUAL(file2_ino, file1_ino);

    // verify file1_ino link count 2
    int file1_nlink = fs->inodes[file1_ino].nlink;
    CU_ASSERT_EQUAL(file1_nlink, 2);


    /// ensure cannot create link for directory inode

    // create "dir1"
    int dir1_ino = fs_mkdir(fs, fs->root_inode, "dir1", dir_mode);
    CU_ASSERT_TRUE(dir1_ino > 0);

    // ensure cannot create "dir1" link
    int dir2_ino = fs_mklink(fs, fs->root_inode, dir1_ino, "dir2");
    CU_ASSERT_FALSE(dir2_ino > 0);


    /// ensure can create link within subdirectory inode

    // create "file1" link in "dir1"
    int dir1_file1_ino = fs_mklink(fs, dir1_ino, file1_ino, "file1");
    CU_ASSERT_TRUE(dir1_file1_ino > 0);

    // verify file1_ino matches dir1_file1_ino
    CU_ASSERT_EQUAL(file1_ino, dir1_file1_ino);

    // verify dir1_file1_ino link count 3
    int dir1_file1_nlink = fs->inodes[dir1_file1_ino].nlink;
    CU_ASSERT_EQUAL(dir1_file1_nlink, 3);

    // unlink "file1" from dir1_ino
    int dir1_file1_status = fs_unlinkfile(fs, dir1_ino, "file1");
    CU_ASSERT_EQUAL(dir1_file1_status, 0);

    // ensure file1_ino is still allocated
    int file1_ino_set = FD_ISSET(file1_ino, fs->inode_map);
    CU_ASSERT_NOT_EQUAL(file1_ino_set, 0);

    // verify dir1_file1_ino link count 2
    file1_nlink = fs->inodes[file1_ino].nlink;
    CU_ASSERT_EQUAL(file1_nlink, 2);

    // ensure cannot unlink file1_ino from dir1_ino
    dir1_file1_status = fs_unlinkfile(fs, dir1_ino, "file1");
    CU_ASSERT_NOT_EQUAL(dir1_file1_status, 0);

    // ensure file1_ino is still allocated
    file1_ino_set = FD_ISSET(file1_ino, fs->inode_map);
    CU_ASSERT_NOT_EQUAL(file1_ino_set, 0);

    // ensure file2_ino link count 2
    int file2_nlink = fs->inodes[file2_ino].nlink;
    CU_ASSERT_EQUAL(file2_nlink, 2);

    // unlink file2_ino from root directory
    int file2_status = fs_unlinkfile(fs, fs->root_inode, "file2");
    CU_ASSERT_EQUAL(file2_status, 0);

    // unlink file1_ino from root directory
    int file1_status = fs_unlinkfile(fs, fs->root_inode, "file1");
    CU_ASSERT_EQUAL(file1_status, 0);

    // ensure file1_ino not allocated
    file1_ino_set = FD_ISSET(file1_ino, fs->inode_map);
    CU_ASSERT_EQUAL(file1_ino_set, 0);

    // unmount file system volume and close device
    fs_unmount_volume(fs);
    dev->ops->close(dev);
}

/**
 * Test file system read directory operations.
 */
void test_readdir(void) {
    const int n_blks = 100;
    const mode_t file_mode = 0644;  // rw-r--r--
    const mode_t dir_mode = 0755;   // rwxr-xr-xx

    // create memory block device
    struct fs_dev_blkdev *dev = memory_blkdev_create(n_blks);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev);

    // format volume
    int fmtstatus = fs_format_volume(dev, 0, 0);
    CU_ASSERT_EQUAL_FATAL(fmtstatus, 0);

    // mount formatted volume
    struct fs_ext2 *fs = fs_mount_volume(dev);
    CU_ASSERT_PTR_NOT_NULL_FATAL(fs);

    /// create file1, file2, dir1

    // create "file1"
    int file1_ino = fs_mkfile(fs, fs->root_inode, "file1", file_mode);
    CU_ASSERT_TRUE_FATAL(file1_ino > 0);

    // create "File2"
    int file2_ino = fs_mkfile(fs, fs->root_inode, "file2", file_mode);
    CU_ASSERT_TRUE_FATAL(file2_ino > 0);

    // create "dir1"
    int dir1_ino = fs_mkdir(fs, fs->root_inode, "dir1", dir_mode);
    CU_ASSERT_TRUE(dir1_ino > 0);

    /// create directory stream and read to completion

    // ensure created directory stream
    FS_DIR *dirp = fs_opendir(fs, fs->root_inode);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dirp);

    // read "."
    struct fs_dirent *de = fs_readdir(dirp);
    CU_ASSERT_PTR_NOT_NULL_FATAL(de);
    CU_ASSERT_STRING_EQUAL(de->name, ".");
    CU_ASSERT_TRUE(de->isDir);

    // read ".."
    de = fs_readdir(dirp);
    CU_ASSERT_PTR_NOT_NULL_FATAL(de);
    CU_ASSERT_STRING_EQUAL(de->name, "..");
    CU_ASSERT_TRUE(de->isDir);

    // read "file1"
    de = fs_readdir(dirp);
    CU_ASSERT_PTR_NOT_NULL_FATAL(de);
    CU_ASSERT_STRING_EQUAL(de->name, "file1");
    CU_ASSERT_FALSE(de->isDir);

    // read "file2"
    de = fs_readdir(dirp);
    CU_ASSERT_PTR_NOT_NULL_FATAL(de);
    CU_ASSERT_STRING_EQUAL(de->name, "file2");
    CU_ASSERT_FALSE(de->isDir);

    // read "dir1"
    de = fs_readdir(dirp);
    CU_ASSERT_PTR_NOT_NULL_FATAL(de);
    CU_ASSERT_STRING_EQUAL(de->name, "dir1");
    CU_ASSERT_TRUE(de->isDir);

    // read end of stream
    de = fs_readdir(dirp);
    CU_ASSERT_PTR_NULL_FATAL(de);

    // close directory stream
    fs_closedir(dirp);
}

/**
 * Test file system i/o operations.
 */
void test_io(void) {
    const int n_blks = 100;
    const mode_t file_mode = 0644;  // rw-r--r--

    // create memory block device
    struct fs_dev_blkdev *dev = memory_blkdev_create(n_blks);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev);

    // format volume
    int fmtstatus = fs_format_volume(dev, 0, 0);
    CU_ASSERT_EQUAL_FATAL(fmtstatus, 0);

    // mount formatted volume
    struct fs_ext2 *fs = fs_mount_volume(dev);
    CU_ASSERT_PTR_NOT_NULL_FATAL(fs);

    // create "file1"
    int file1_ino = fs_mkfile(fs, fs->root_inode, "file1", file_mode);
    CU_ASSERT_TRUE(file1_ino > 0);

    // verify file1 is empty
    CU_ASSERT_EQUAL(fs->inodes[file1_ino].size, 0);

    // get file1 mod time for comparison
    int file1_time = fs->inodes[file1_ino].mtime;
    sleep(1); // time passes

    char msg[2*FS_BLOCK_SIZE];
    memset(msg, 'a', 2*FS_BLOCK_SIZE);

    // write message to file1
    int msglen = FS_BLOCK_SIZE;
    int status = fs_writefile(fs, file1_ino, msg, msglen);
    // ensure successful write
    CU_ASSERT_EQUAL(status, 0);
    // ensure inode size matches
    CU_ASSERT_EQUAL(fs->inodes[file1_ino].size, msglen);
    // ensure mode time has changed
    CU_ASSERT_NOT_EQUAL(fs->inodes[file1_ino].mtime, file1_time);

    // short read from file1
    char readbuf[FS_BLOCK_SIZE] = "bbbbbbbbbb";
    int readmsglen = 5;
    int nread = fs_readfile(fs, file1_ino, readbuf, readmsglen);
    // ensure read back size matches
    CU_ASSERT_EQUAL(nread, readmsglen);
    // ensure read back original message
    CU_ASSERT_STRING_EQUAL(readbuf, "aaaaabbbbb");

    // write content too large
    // NOTE: current implementation supports
    // only 1 block of storage.
    int bigmsglen = 2*FS_BLOCK_SIZE;
    status = fs_writefile(fs, file1_ino, msg, bigmsglen);
    // ensure write failed -- too large
    CU_ASSERT_NOT_EQUAL(status, 0);
    // ensure inode size has not changed
    CU_ASSERT_EQUAL(fs->inodes[file1_ino].size, msglen);

    // unmount file system volume and close device
    fs_unmount_volume(fs);
    dev->ops->close(dev);
}

/**
 * Test file system truncate operations.
 */
void test_trunc(void) {
    const int n_blks = 100;
    const mode_t file_mode = 0644;  // rw-r--r--

    // create memory block device
    struct fs_dev_blkdev *dev = memory_blkdev_create(n_blks);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev);

    // format volume
    int fmtstatus = fs_format_volume(dev, 0, 0);
    CU_ASSERT_EQUAL_FATAL(fmtstatus, 0);

    // mount formatted volume
    struct fs_ext2 *fs = fs_mount_volume(dev);
    CU_ASSERT_PTR_NOT_NULL_FATAL(fs);

    // create "file1"
    int file1_ino = fs_mkfile(fs, fs->root_inode, "file1", file_mode);
    CU_ASSERT_TRUE(file1_ino > 0);

    char msg[FS_BLOCK_SIZE];
    memset(msg, 'a', FS_BLOCK_SIZE);

    // write message to file1
    int status = fs_writefile(fs, file1_ino, msg, FS_BLOCK_SIZE);
    // ensure successful write
    CU_ASSERT_EQUAL(status, 0);
    CU_ASSERT_EQUAL(fs->inodes[file1_ino].size, FS_BLOCK_SIZE);

    // get file1 mod time for comparison
    int file1_time = fs->inodes[file1_ino].mtime;
    sleep(1); // time passes

    // truncate to same length
    int trunc_status = fs_truncfile(fs, file1_ino, FS_BLOCK_SIZE);
    CU_ASSERT_EQUAL(trunc_status, 0);

    // ensure inode size matches
    CU_ASSERT_EQUAL(fs->inodes[file1_ino].size, FS_BLOCK_SIZE);
    // ensure mode time has not changed
    CU_ASSERT_EQUAL(fs->inodes[file1_ino].mtime, file1_time);

    // get file1 mod time for comparison
    file1_time = fs->inodes[file1_ino].mtime;
    sleep(1); // time passes

    // truncate to 1 byte
    trunc_status = fs_truncfile(fs, file1_ino, 1);
    CU_ASSERT_EQUAL(trunc_status, 0);

    // ensure inode size matches
    CU_ASSERT_EQUAL(fs->inodes[file1_ino].size, 1);
    // ensure mode time has changed
    CU_ASSERT_NOT_EQUAL(fs->inodes[file1_ino].mtime, file1_time);

    // truncate to 3 bytes
    trunc_status = fs_truncfile(fs, file1_ino, 3);
    CU_ASSERT_EQUAL(trunc_status, 0);

    // ensure inode size matches
    CU_ASSERT_EQUAL(fs->inodes[file1_ino].size, 3);
    // ensure mode time has changed
    CU_ASSERT_NOT_EQUAL(fs->inodes[file1_ino].mtime, file1_time);

    // read block
    int read_size = fs_readfile(fs, file1_ino, msg, FS_BLOCK_SIZE);
    CU_ASSERT_EQUAL(read_size, 3);
    // other bytes 0'd
    CU_ASSERT_STRING_EQUAL(msg, "a");

    // unmount file system volume and close device
    fs_unmount_volume(fs);
    dev->ops->close(dev);
}

/**
 * Test file system stat operations.
 */
void test_stat(void) {
    const int n_blks = 100;
    const mode_t file_mode = 0644;  // rw-r--r--

    // create memory block device
    struct fs_dev_blkdev *dev = memory_blkdev_create(n_blks);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev);

    // format volume
    int fmtstatus = fs_format_volume(dev, 0, 0);
    CU_ASSERT_EQUAL_FATAL(fmtstatus, 0);

    // mount formatted volume
    struct fs_ext2 *fs = fs_mount_volume(dev);
    CU_ASSERT_PTR_NOT_NULL_FATAL(fs);

    // create "file1"
    int file1_ino = fs_mkfile(fs, fs->root_inode, "file1", file_mode);
    CU_ASSERT_TRUE(file1_ino > 0);

    char msg[FS_BLOCK_SIZE];
    memset(msg, 'a', FS_BLOCK_SIZE);

    // write message to file1
    int status = fs_writefile(fs, file1_ino, msg, FS_BLOCK_SIZE-1);
    // ensure successful write
    CU_ASSERT_EQUAL(status, 0);

    struct stat sb;
    fs_stat(fs, file1_ino, &sb);

    // test selection of struct stat fields
    CU_ASSERT_EQUAL(sb.st_ino, file1_ino);
    CU_ASSERT_EQUAL(sb.st_blksize, FS_BLOCK_SIZE);
    CU_ASSERT_EQUAL(sb.st_size, FS_BLOCK_SIZE-1);
    CU_ASSERT_EQUAL(sb.st_blocks, 2);  // 2 512 byte blocks
    CU_ASSERT_EQUAL(sb.st_nlink, 1);
    CU_ASSERT_EQUAL(sb.st_atime, sb.st_mtime);

    // unmount file system volume and close device
    fs_unmount_volume(fs);
    dev->ops->close(dev);
}

/**
 * Test file system statfs operations.
 */
void test_statfs(void) {
    const int n_blks = 100;
    const mode_t file_mode = 0644;  // rw-r--r--

    // create memory block device
    struct fs_dev_blkdev *dev = memory_blkdev_create(n_blks);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev);

    // format volume
    int fmtstatus = fs_format_volume(dev, 0, 0);
    CU_ASSERT_EQUAL_FATAL(fmtstatus, 0);

    // mount formatted volume
    struct fs_ext2 *fs = fs_mount_volume(dev);
    CU_ASSERT_PTR_NOT_NULL_FATAL(fs);

    struct statvfs sb;
    fs_statfs(fs, &sb);

    // test selection of struct statvfs fields
    CU_ASSERT_EQUAL(sb.f_blocks, fs->n_blocks);
    CU_ASSERT_EQUAL(sb.f_bsize, FS_BLOCK_SIZE);
    CU_ASSERT_EQUAL(sb.f_files, fs->n_inodes);
    CU_ASSERT_EQUAL(sb.f_namemax, FS_FILENAME_SIZE-1);

    // unmount file system volume and close device
    fs_unmount_volume(fs);
    dev->ops->close(dev);
}

/**
 * Test file system sync meta operations.
 */
static void test_sync_meta(void) {
    const int n_blks = 100;
    const mode_t file_mode = 0644;  // rw-r--r--
    const mode_t dir_mode = 0755;   // rwxr-xr-xx

    // create memory block device
    struct fs_dev_blkdev *dev = memory_blkdev_create(n_blks);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev);

    // format volume
    int fmtstatus = fs_format_volume(dev, 0, 0);
    CU_ASSERT_EQUAL_FATAL(fmtstatus, 0);

    // mount formatted volume
    struct fs_ext2 *fs = fs_mount_volume(dev);
    CU_ASSERT_PTR_NOT_NULL_FATAL(fs);

    // create "file1"
    int file1_ino = fs_mkfile(fs, fs->root_inode, "file1", file_mode);
    CU_ASSERT_TRUE(file1_ino > 0);

    // create "file2"
    int file2_ino = fs_mklink(fs, fs->root_inode, file1_ino, "file2");
    CU_ASSERT_TRUE(file2_ino > 0);

    // create "dir1"
    int dir1_ino = fs_mkdir(fs, fs->root_inode, "dir1", dir_mode);
    CU_ASSERT_TRUE(dir1_ino > 0);

    // verify inodes are allocated
    CU_ASSERT_NOT_EQUAL(FD_ISSET(file1_ino, fs->inode_map), 0);
    CU_ASSERT_NOT_EQUAL(FD_ISSET(file2_ino, fs->inode_map), 0);
    CU_ASSERT_NOT_EQUAL(FD_ISSET(dir1_ino, fs->inode_map), 0);

    // unmount and re-mount volume to rebuild fs
    fs_unmount_volume(fs);
    fs = fs_mount_volume(dev);
    CU_ASSERT_PTR_NOT_NULL_FATAL(fs);

    // ensure inodes are still allocated
    CU_ASSERT_NOT_EQUAL(FD_ISSET(file1_ino, fs->inode_map), 0);
    CU_ASSERT_NOT_EQUAL(FD_ISSET(file2_ino, fs->inode_map), 0);
    CU_ASSERT_NOT_EQUAL(FD_ISSET(dir1_ino, fs->inode_map), 0);

    // unmount file system volume and close device
    fs_unmount_volume(fs);
    dev->ops->close(dev);
}

/*++ Optional function tests ++*/

/**
 * Test file system pread/pwrite operations.
 */
void test_pio(void) {
    const int n_blks = 100;
    const mode_t file_mode = 0644;  // rw-r--r--

    // create memory block device
    struct fs_dev_blkdev *dev = memory_blkdev_create(n_blks);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev);

    // format volume
    int fmtstatus = fs_format_volume(dev, 0, 0);
    CU_ASSERT_EQUAL_FATAL(fmtstatus, 0);

    // mount formatted volume
    struct fs_ext2 *fs = fs_mount_volume(dev);
    CU_ASSERT_PTR_NOT_NULL_FATAL(fs);

    // create "file1"
    int file1_ino = fs_mkfile(fs, fs->root_inode, "file1", file_mode);
    CU_ASSERT_TRUE(file1_ino > 0);

    // verify file1 is empty
    CU_ASSERT_EQUAL(fs->inodes[file1_ino].size, 0);

    // get file1 mod time for comparison
    int file1_time = fs->inodes[file1_ino].mtime;
    sleep(1); // time passes

    // initialize block with half 'a's and half 'b's
    const int halfblock = FS_BLOCK_SIZE/2;
    char msg[FS_BLOCK_SIZE];
    char *msga = msg;
    char *msgb = msg+halfblock;
    memset(msga, 'a', halfblock);
    memset(msgb, 'b', halfblock);

    // write 'b's to upper-half block of file1
    int status = fs_pwritefile(fs, file1_ino, msgb, halfblock, halfblock);
    // ensure successful write
    CU_ASSERT_EQUAL(status, 0);
    // ensure inode size matches
    CU_ASSERT_EQUAL(fs->inodes[file1_ino].size, FS_BLOCK_SIZE);
    // ensure mode time has changed
    CU_ASSERT_NOT_EQUAL(fs->inodes[file1_ino].mtime, file1_time);

    // read upper-half block written to file1
    char readbuf[FS_BLOCK_SIZE];
    int nread = fs_preadfile(fs, file1_ino, readbuf, halfblock, halfblock);
    // ensure read successful
    CU_ASSERT_EQUAL(nread, halfblock);
    // ensure bytes match
    int cmp = (bcmp(msgb, readbuf, nread) == 0);
    CU_ASSERT_EQUAL(cmp, 1);

    // read lower-half block
    nread = fs_preadfile(fs, file1_ino, readbuf, halfblock, 0);
    // ensure read successful
    CU_ASSERT_EQUAL(nread, halfblock);
    // ensure lower-half block is padded with 0s -- trick compares buf with shifted buf
    cmp =  (readbuf[0] == 0) && (memcmp(readbuf, readbuf+1, halfblock-1) == 0);
    CU_ASSERT_EQUAL(cmp, 1);

    // write 'a's to lower block
    status = fs_pwritefile(fs, file1_ino, msga, halfblock, 0);
    // ensure successful write
    CU_ASSERT_EQUAL(status, 0);
    // ensure inode size matches
    CU_ASSERT_EQUAL(fs->inodes[file1_ino].size, FS_BLOCK_SIZE);

    // read entire block
    nread = fs_preadfile(fs, file1_ino, readbuf, FS_BLOCK_SIZE, 0);
    // ensure read successful
    CU_ASSERT_EQUAL(nread, FS_BLOCK_SIZE);
    // ensure lower-half padded with 'a's, upper half with 'b's
    cmp = (bcmp(msg, readbuf, FS_BLOCK_SIZE) == 0);
    CU_ASSERT_EQUAL(cmp, 1);

    // unmount file system volume and close device
    fs_unmount_volume(fs);
    dev->ops->close(dev);
}

/**
 * Test file system chown/chmod/utime operations.
 */
void test_chown_chmod_utime(void) {
    const int n_blks = 100;
    const mode_t file_mode = 0644;  // rw-r--r--

    // create memory block device
    struct fs_dev_blkdev *dev = memory_blkdev_create(n_blks);
    CU_ASSERT_PTR_NOT_NULL_FATAL(dev);

    // format volume
    int fmtstatus = fs_format_volume(dev, 0, 0);
    CU_ASSERT_EQUAL_FATAL(fmtstatus, 0);

    // mount formatted volume
    struct fs_ext2 *fs = fs_mount_volume(dev);
    CU_ASSERT_PTR_NOT_NULL_FATAL(fs);

    // create "file1"
    int file1_ino = fs_mkfile(fs, fs->root_inode, "file1", file_mode);
    CU_ASSERT_TRUE(file1_ino > 0);

    // verify file1 is empty
    CU_ASSERT_EQUAL(fs->inodes[file1_ino].size, 0);

    // set file1 user to 1 and group to 2
    int status = fs_chown(fs, file1_ino, 1, 2);
    CU_ASSERT_EQUAL(status, 0);
    CU_ASSERT_EQUAL(fs->inodes[file1_ino].uid, 1);
    CU_ASSERT_EQUAL(fs->inodes[file1_ino].gid, 2);

    // set file1 access mode to 0111
    status = fs_chmod(fs, file1_ino, 0111);
    CU_ASSERT_EQUAL(status, 0);
    CU_ASSERT_EQUAL(fs->inodes[file1_ino].mode & 0777, 0111);

    // set file1 mod time to 1
    status = fs_utime(fs, file1_ino, 1);
    CU_ASSERT_EQUAL(status, 0);
    CU_ASSERT_EQUAL(fs->inodes[file1_ino].mtime, 1);

    // unmount file system volume and close device
    fs_unmount_volume(fs);
    dev->ops->close(dev);
}

/**
 * Test all the functions for this application.
 *
 * @return test error code
 */
static CU_ErrorCode test_all() {
    // initialize the CUnit test registry -- only once per application
    CU_initialize_registry();

    // add a suite to the registry with no init or cleanup
    CU_pSuite pSuite = CU_add_suite("word_tests", NULL, NULL);

    // add the tests to the suite
    CU_add_test(pSuite, "test_mem_device", test_mem_device);
    CU_add_test(pSuite, "test_volume", test_volume);
    CU_add_test(pSuite, "test_file_cpcd", test_file_cpcd);
    CU_add_test(pSuite, "test_file_cpci", test_file_cpci);
    CU_add_test(pSuite, "test_dir", test_dir);
    CU_add_test(pSuite, "test_link", test_link);
    CU_add_test(pSuite, "test_readdir", test_readdir);
    CU_add_test(pSuite, "test_io", test_io);
    CU_add_test(pSuite, "test_trunc", test_trunc);
    CU_add_test(pSuite, "test_stat", test_stat);
    CU_add_test(pSuite, "test_statfs", test_statfs);
    CU_add_test(pSuite, "test_sync_meta", test_sync_meta);

    // add thee optional function tests to the suite
    CU_add_test(pSuite, "test_pio", test_pio);
    CU_add_test(pSuite, "test_chown_chmod_utime", test_chown_chmod_utime);

    // run all test suites using the basic interface
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    // display information on failures that occurred
    CU_basic_show_failures(CU_get_failure_list());

    // Clean up registry and return status
    CU_cleanup_registry();
    return CU_get_error();
}

#include <sys/stat.h>
/**
 * Main program to invoke test functions
 *
 * @return the exit status of the program
 */
int main(void) {
    // test all the functions
    CU_ErrorCode code = test_all();

    return (code == CUE_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}
