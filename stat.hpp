#pragma once

enum InodeType
{
    T_DIR  = 1, /// Directory
    T_FILE = 2, /// File
    T_DEV  = 3, /// Device
};

struct stat
{
    enum InodeType type;  /// Type of file
    int            dev;   /// File system's disk device
    uint           ino;   /// Inode number
    short          nlink; /// Number of links to file
    uint           size;  /// Size of file in bytes
};
