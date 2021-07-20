cmake_minimum_required(VERSION 3.17)
project(assignment_4 C)

set(CMAKE_C_STANDARD 11)

#definitions required by FS implementation
#  FS_VERSION=0 -- no links or ".", ".." entries
#  FS_VERSION=1 -- with links and ".", ".." entries
#  FS_VERSION=2 -- with symlinks
add_compile_definitions(FS_VERSION=1)

# paths for cunit
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_compile_definitions(_DARWIN_UNLIMITED_SELECT)

add_executable(assignment_4
        assignment-4-test.c
        fs_dev_memorydev.c
        fs_op_chmodfile.c
        fs_op_chownfile.c
        fs_op_mkfile.c
        fs_op_readdir.c
        fs_op_readfile.c
        fs_op_statfile.c
        fs_op_statfs.c
        fs_op_truncfile.c
        fs_op_unlinkfile.c
        fs_op_utimefile.c
        fs_op_writefile.c
        fs_util_format.c
        fs_util_volume.c
        )
target_link_libraries(assignment_4 cunit)
