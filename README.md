# File System Simulation
## Introduction
In this assignment, you will have an opportunity to implement operations on a file system that use a simulated memory-based block device. The fsx600 file system follows the design described in lecture 8, with a superblock for the volume, inode and block bitmaps to track free inodes and blocks, inodes that represent files and directories, and a pool of blocks that can be allocated and used.

## Supplemental Information
The PDF document "Assignment 4 – File System Implementation"PDF document" provides details of the design of the file system described in the lecture 8 presentation, including a description of the major data structures. The design is based on Ext2 -- Second Extended File System, a popular file and directory system design that has been in wide use, and serves as the base for Ext3 and other more advanced versions. This document also provides a summary of the file system functions that you will implement.

## Starting Code
Source files contains code for a simulated memory-based block device and for formatting and mounting the file system. It also includes stubs for file system functions that you will implement. In some cases, a group of functions are provided that use a common supporting function that you will implement. For example, the functions to create a file, directory, and a hard link have a common supporting function, as do the functions to unlink a file and remove a directory. Each stub function provides a description of how to implement it. Some supporting functions are also provided that will be useful for implementing the stub functions.

Finally, the repository includes a unit test program that exercises the block device and file system functions, including the ones you will implement. The tests for the block device and for formatting and mounting the file system volume will work directly. The other tests will fail until you have implemented the file system functions being tested. A recommendation is to comment out the code that registers these tests and uncomment them once you are ready to test those functions.

For those who are using CLion for development, the repository includes a CMakeLists.txt file with a configuration for building and running the unit test application.
