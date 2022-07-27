#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int 
sys_getyear(void)
{
  return 1975;
}

// implemented getprocinfo system call
int 
sys_getprocinfo(void)
{
  struct uproc* up;
  if(argptr(0, (char**)&up, sizeof(up)) < 0) // Arguments taking from stack to store details in up
    return -1;
  return getprocinfo(up);
}

// savehistory system call created because sh.c is using them, so it has to make system call to access kernel things
int 
sys_savehistory(void)
{
  char* cmd;
  if(argptr(0, &cmd, sizeof(cmd)) < 0)
    return -1;
  savehistory(cmd);
  return 0;
}

// gethistory system call because sh.c is using them, so it has to make system call to access kernel things
int 
sys_gethistory(void)
{
  char* history;
  if(argptr(0, (char **)&history, sizeof(history)) < 0)
    return -1;
  // cprintf("sysproc.c %d:%p\n",__LINE__,history); // debugging
  return gethistory(history);
}

// implemented wait, used in statistics 
int 
sys_wait2(void){
  int *ctime, *retime, *rutime, *stime, *ttime;
  if(argptr(0, (char **)&ctime, sizeof(ctime)) < 0) 
    return -1;
  if(argptr(1, (char **)&retime, sizeof(ctime)) < 0) 
    return -1;
  if(argptr(2, (char **)&rutime, sizeof(ctime)) < 0) 
    return -1;
  if(argptr(3, (char **)&stime, sizeof(ctime)) < 0) 
    return -1;
  if(argptr(4, (char **)&ttime, sizeof(ctime)) < 0) 
    return -1;
  return wait2(ctime, retime, rutime, stime, ttime);
}