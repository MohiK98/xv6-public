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
sys_uptime
(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);

  return xticks;
}

void
sys_inc_num(void)
{
  int n;
  // argint(0, &n);
  n = myproc()->tf->ebx;

  return inc_num(n);
}

void
sys_invoked_syscalls(void)
{
  int pid;
  argint(0, &pid);

  return invoked_syscalls(pid);
}

void
sys_sort_syscalls(void)
{
  int pid;
  argint(0, &pid);

  return sort_syscalls(pid);
}

void
sys_get_count(void)
{
  int pid, num;
  argint(0, &pid);
  argint(1, &num);

  return get_count(pid, num);
}

void
sys_log_syscalls(void)
{
  return log_syscalls();
}

void sys_ticketlockinit(void)
{
  return ticketlockinit();
}

void sys_ticketlocktest(void)
{
  return ticketlocktest();
}

void sys_rwinit(void)
{
  return rwinit();
}

void sys_rwtest(void)
{
  return rwtest();
}