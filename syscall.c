#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"
#include "date.h"
#include "stat.h"

#define NUMBER 1
#define STRING 2
#define POINTER 3

// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.

// Fetch the int at addr from the current process.
int
fetchint(uint addr, int *ip)
{
  struct proc *curproc = myproc();

  if(addr >= curproc->sz || addr+4 > curproc->sz)
    return -1;
  *ip = *(int*)(addr);
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int
fetchstr(uint addr, char **pp)
{
  char *s, *ep;
  struct proc *curproc = myproc();

  if(addr >= curproc->sz)
    return -1;
  *pp = (char*)addr;
  ep = (char*)curproc->sz;
  for(s = *pp; s < ep; s++){
    if(*s == 0)
      return s - *pp;
  }
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  return fetchint((myproc()->tf->esp) + 4 + 4*n, ip);
}
// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size bytes.  Check that the pointer
// lies within the process address space.
int
argptr(int n, char **pp, int size)
{
  int i;
  struct proc *curproc = myproc();
 
  if(argint(n, &i) < 0)
    return -1;
  if(size < 0 || (uint)i >= curproc->sz || (uint)i+size > curproc->sz)
    return -1;
  *pp = (char*)i;
  return 0;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int
argstr(int n, char **pp)
{
  int addr;
  if(argint(n, &addr) < 0)
    return -1;
  return fetchstr(addr, pp);
}

extern int sys_chdir(void);
extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_fstat(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_link(void);
extern int sys_mkdir(void);
extern int sys_mknod(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_unlink(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);
extern int sys_inc_num(void);
extern int sys_invoked_syscalls(void);
extern int sys_sort_syscalls(void);
extern int sys_get_count(void);
extern int sys_log_syscalls(void);
extern int sys_ticketlockinit(void);
extern int sys_ticketlocktest(void);
extern int sys_rwinit(void);
extern int sys_rwtest(void);
extern int sys_dealloc(void);
extern int sys_ps(void);

static int (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,
[SYS_inc_num] sys_inc_num,
[SYS_invoked_syscalls] sys_invoked_syscalls,
[SYS_sort_syscalls] sys_sort_syscalls,
[SYS_get_count] sys_get_count,
[SYS_log_syscalls] sys_log_syscalls,
[SYS_ticketlockinit] sys_ticketlockinit,
[SYS_ticketlocktest] sys_ticketlocktest ,
[SYS_rwinit] sys_rwinit ,
[SYS_rwtest] sys_rwtest ,
[SYS_dealloc] sys_dealloc,
[SYS_ps] sys_ps,
};


void fill_arguments(struct syscall_info *s, int num)
{
  int n1, n2, n;
  char* c1;
  char* c2;
  void* p1 = 0;
  char *p;
  int f;
  int i;
  uint uargv;
  uint uarg;
  int* fd;
  char *argv[MAXARG];
  switch(num)
  {
    case 1:
      s->arg_count = 0;
      break;
    case 2:
      s->arg_count = 0;
      break;
    case 3:
      s->arg_count = 0;
      break;
    case 4:
      s->arg_count = 1;
      argptr(0, (void*)&fd, 2*sizeof(fd[0]));
      s->args[0].pointer = fd;
      s->args[0].type = POINTER;
      break;
    case 5:
      argint(0, &f);
      argint(2, &n);
      argptr(1, &p, n);
      s->arg_count = 3;
      s->args[0].value = n;
      s->args[0].type = NUMBER;
      s->args[1].pointer = p;
      s->args[1].type = POINTER;
      s->args[2].value = f;
      s->args[2].type = NUMBER;
      break;
    case 6:
      s->arg_count = 1;
      argint(0, &n);
      s->args[0].value = n;
      s->args[0].type = NUMBER;
      break;
    case 7:
      s->arg_count = 2;
      argstr(0, &c1);
      char* temp = c1;
      int n;
      s->args[0].string = (char*)kalloc();
      memset(s->args[0].string, 0, 30);
      for(n = 0; temp[n]; n++){
         *(s->args[0].string + n) = *(temp + n);
         
      }
      argint(1, (int*)&uargv);
      memset(argv, 0, sizeof(argv));
      for(i=0;; i++){
        fetchint(uargv+4*i, (int*)&uarg);
        if(uarg == 0){
          argv[i] = 0;
          break;
        }
      }
      fetchstr(uarg, &argv[i]);
      s->args[0].type = STRING;
      s->args[1].pointer = argv;
      s->args[1].type = POINTER;
      break;
    case 8:
      s->arg_count = 2;
      argint(0, &n1);
      argptr(1, (void*)p1, sizeof(*p1));
      s->args[0].value = n1;
      s->args[0].type = NUMBER;
      s->args[1].pointer = p1;
      s->args[1].type = POINTER;
      break;
    case 9:
      s->arg_count = 1;
      argstr(0, &c1);
      s->args[0].string = c1;
      s->args[0].type = STRING;
      break;
    case 10:
      s->arg_count = 1;
      argint(0, &n);
      s->args[0].value = n;
      s->args[0].type = NUMBER;
      break;
    case 11:
      s->arg_count = 0;
      break;
    case 12:
      s->arg_count = 1;
      argint(0, &n);
      s->args[0].value = n;
      s->args[0].type = NUMBER;
      break;
    case 13:
      s->arg_count = 1;
      argint(0, &n);
      s->args[0].value = n;
      s->args[0].type = NUMBER;
      break;
    case 14:
      s->arg_count = 0;
      break;
    case 15:
      s->arg_count = 2;
      argstr(0, &c1);
      argint(1, &n);
      s->args[0].string = c1;
      s->args[0].type = STRING;
      s->args[1].value = n;
      s->args[1].type = NUMBER ;
      break;
    case 16:
      argint(0, &f);
      argint(2, &n);
      argptr(1, &p, n);
      s->arg_count = 3;
      s->args[0].value = n;
      s->args[0].type = NUMBER;
      s->args[1].pointer = p;
      s->args[1].type = POINTER;
      s->args[2].value = f;
      s->args[2].type = NUMBER;
      break;
    case 17:
      argstr(0, &c1);
      argint(1, &n1);
      argint(2, &n2);
      s->arg_count = 3;
      s->args[0].string = c1;
      s->args[0].type = STRING;
      s->args[1].value = n1;
      s->args[1].type = NUMBER;
      s->args[2].value = n2;
      s->args[2].type = NUMBER;
      break;
    case 18:
      s->arg_count = 1;
      argstr(0, &c1);
      s->args[0].string = c1;
      s->args[0].type = STRING;
      break;
    case 19:
      s->arg_count = 2;
      argstr(0, &c1);
      argstr(1, &c2);
      s->args[0].string = c1;
      s->args[0].type = STRING;
      s->args[1].string = c2;
      s->args[1].type = STRING;
      break;
    case 20:
      s->arg_count = 1;
      argstr(0, &c1);
      s->args[0].string = c1;
      s->args[0].type = STRING;
      break;
    case 21:
      s->arg_count = 1;
      argint(0, &n);
      s->args[0].value = n;
      s->args[0].type = NUMBER;
      break;
    case 22:
      s->arg_count = 1;
      argint(0, &n);
      s->args[0].value = n;
      s->args[0].type = NUMBER;
      break;
    case 23:
      s->arg_count = 1;
      argint(0, &n);
      s->args[0].value = n;
      s->args[0].type = NUMBER;
      break;
    case 24:
      s->arg_count = 1;
      argint(0, &n);
      s->args[0].value = n;
      s->args[0].type = NUMBER;
      break;
    case 25:
      s->arg_count = 2;
      argint(0, &n1);
      argint(1, &n2);
      s->args[0].value = n1;
      s->args[0].type = NUMBER;
      s->args[1].value = n2;
      s->args[1].type = NUMBER;
      break;
    case 26:
      s->arg_count = 0;
      break;
    
  }

}

void
syscall(void)
{
  int num;
  struct proc *curproc = myproc();
  
  num = curproc->tf->eax;

  
  int i;
  for(i=0 ; i< process_details_counter ; i++){
    if (process_details[i].pid == curproc->pid){
      struct syscall_info* syscall_struct = &process_details[i].syscall_det[process_details[i].counter];
      syscall_struct->number = num;
      syscall_struct->name = syscall_arr[num-1];
      syscall_struct->pid = curproc->pid;
      struct rtcdate r;
      cmostime(&r);
      syscall_struct->t = r;
      fill_arguments(syscall_struct, num);
      sys_log[sys_log_counter] = syscall_struct;
      sys_log_counter ++;
      process_details[i].counter += 1;
      break;
    }
  }


  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    curproc->tf->eax = syscalls[num]();
  } else {
    cprintf("%d %s: unknown sys call %d\n",
            curproc->pid, curproc->name, num);
    curproc->tf->eax = -1;
  }

}




// Fetch the nth 32-bit system call argument.
// int
// argint(int n, int *ip)
// {
//   return fetchint((myproc()->tf->esp) + 4 + 4*n, ip);
// }
