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
  int pid = myproc()->pid;
  process_details[pid].syscall_det[process_details[pid].counter].arg_count = 0;
  return fork();
}

int
sys_exit(void)
{
  int pid = myproc()->pid;
  process_details[pid].syscall_det[process_details[pid].counter].arg_count = 0;
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  int pid = myproc()->pid;
  process_details[pid].syscall_det[process_details[pid].counter].arg_count = 0;
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  struct syscall_info *si = &process_details[pid].syscall_det[process_details[pid].counter];
  si->arg_count = 1;
  si->args[0].value = pid;
  si->args[0].type = NUMBER;
  return kill(pid);
}

int
sys_getpid(void)
{
  int pid = myproc()->pid;
  struct syscall_info *si = &process_details[pid].syscall_det[process_details[pid].counter];
  si->arg_count = 0;
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
  
  int pid = myproc()->pid;
  struct syscall_info *si = &process_details[pid].syscall_det[process_details[pid].counter];
  si->arg_count = 1;
  si->args[0].value = n;
  si->args[0].type = NUMBER;

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

  int pid = myproc()->pid;
  struct syscall_info *si = &process_details[pid].syscall_det[process_details[pid].counter];
  si->arg_count = 1;
  si->args[0].value = n;
  si->args[0].type = NUMBER;

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

  int pid = myproc()->pid;
  struct syscall_info *si = &process_details[pid].syscall_det[process_details[pid].counter];
  si->arg_count = 0;

  return xticks;
}

void
sys_inc_num(void)
{
  int n;
  argint(0, &n);

  int pid = myproc()->pid;
  struct syscall_info *si = &process_details[pid].syscall_det[process_details[pid].counter];
  si->arg_count = 1;
  si->args[0].value = n;
  si->args[0].type = NUMBER;

  return inc_num(n);
}

void
sys_invoked_syscalls(void)
{
  int pid;
  argint(0, &pid);

  int p = myproc()->pid;
  struct syscall_info *si = &process_details[p].syscall_det[process_details[p].counter];
  si->arg_count = 1;
  si->args[0].value = pid;
  si->args[0].type = NUMBER;


  return invoked_syscalls(pid);
}

void
sys_sort_syscalls(void)
{
  int pid;
  argint(0, &pid);

  int p = myproc()->pid;
  struct syscall_info *si = &process_details[p].syscall_det[process_details[p].counter];
  si->arg_count = 1;
  si->args[0].value = pid;
  si->args[0].type = NUMBER;

  return sort_syscalls(pid);
}

void
sys_get_count(void)
{
  int pid, num;
  argint(0, &pid);
  argint(1, &num);

  int p = myproc()->pid;
  struct syscall_info *si = &process_details[p].syscall_det[process_details[p].counter];
  si->arg_count = 2;
  si->args[0].value = pid;
  si->args[0].type = NUMBER;
  si->args[0].value = num;
  si->args[0].type = NUMBER;

  return get_count(pid, num);
}

void
sys_log_syscalls(void)
{
  int p = myproc()->pid;
  struct syscall_info *si = &process_details[p].syscall_det[process_details[p].counter];
  si->arg_count = 0;
  return log_syscalls();
}