#include "date.h"

void sleepticket(void* chan);

// Per-CPU state
struct cpu {
  uchar apicid;                // Local APIC ID
  struct context *scheduler;   // swtch() here to enter scheduler
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  volatile uint started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?
  struct proc *proc;           // The process running on this cpu or null
};

extern struct cpu cpus[NCPU];
extern int ncpu;

//PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
enum procType {LOTTERY, FCFS, PRIORITY};
 
struct SHM_INFO{
  uint pa;
  int id;
  int flag;
};

// Per-process state
struct proc {
  uint sz;                     // Size of process memory (bytes)
  pde_t* pgdir;                // Page table
  char *kstack;                // Bottom of kernel stack for this process
  enum procstate state;        // Process state
  int pid;                     // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
  // int shared_memory_ids[16];
  int number_of_shared_memories;
  struct SHM_INFO shm_info[16];
};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap

int process_details_counter;

enum argtypes { NUMBER, STRING, POINTER };


struct argument {
  int type;   
  int value;
  char* string;
  void* pointer;
};

struct syscall_info {
  int number;
  int pid;
  char *name;
  struct argument args[3];
  struct rtcdate t;
  int arg_count;
};

struct process_info {
  int pid;
  int counter;
  struct syscall_info syscall_det[128];
};

struct process_info process_details[128];
char* syscall_arr[30];
int syscall_arg_count[30];

struct syscall_info* sys_log[16384];
int sys_log_counter;


enum shm_flag {NOFLAG, ONLY_OWNER_WRITE, ONLY_CHILD_CAN_ATTACH, BOTH_FLAGS};

struct shared_memory {
  int owner_pid;
  int is_valid;
  int id;
  int flag;
  int ref_count;
  int size;
  char* frames[128];
  int frame_counter;
};
