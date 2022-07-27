#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

// different color codes
#define Red "\033[38;5;9m"
#define Green "\033[38;5;46m"
#define Yellow "\033[38;5;11m"
#define Blue "\033[38;5;33m"
#define Magenta "\033[38;5;35m"
#define Cyan "\033[38;5;36m"
#define White "\033[38;5;37m"
#define BrightRed "\033[38;5;91m"
#define BrightGreen "\033[38;5;92m"
#define LightPink "\033[38;5;133m"
#define Reset "\033[0;0m"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void
ls(char *path)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    printf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    printf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    printf(1, Red);
    printf(1, "%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
    printf(1, Reset);
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf(1, "ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf(1, "ls: cannot stat %s\n", buf);
        continue;
      }
      if(st.type == T_FILE){
        printf(1, LightPink);
      }
      else if(st.type == T_DIR) printf(1,Blue);
      else printf(1, Yellow);
      printf(1, "%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
      printf(1, Reset);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    ls(".");
    exit();
  }
  for(i=1; i<argc; i++)
    ls(argv[i]);
  exit();
}
