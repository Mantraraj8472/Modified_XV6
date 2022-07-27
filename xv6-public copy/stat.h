#ifndef __stat_h__
#define __stat_h__
#include "types.h" 
#define T_DIR  1   // Directory
#define T_FILE 2   // File
#define T_DEV  3   // Device

struct stat {
  short type;  // Type of file
  int dev;     // File system's disk device
  uint ino;    // Inode number
  short nlink; // Number of links to file
  uint size;   // Size of file in bytes
};

// defining uproc structure
struct uproc{
  char name[16];
  int pid;
  int ppid;
  int size;
  char state[10];
};

#endif 