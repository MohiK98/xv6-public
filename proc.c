#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "date.h"
#include "ticketlock.h"
#include "syscall.h"
#include "rwlock.h"

#define NUMBER 1
#define STRING 2
#define POINTER 3

void init_syscall_map() {
  sys_log_counter = 0;
  process_details_counter = 0;
  syscall_arr[0] = "fork";
  syscall_arr[1] = "exit";
  syscall_arr[2] = "wait";
  syscall_arr[3] = "pipe";
  syscall_arr[4] = "read";
  syscall_arr[5] = "kill";
  syscall_arr[6] = "exec";
  syscall_arr[7] = "fstat";
  syscall_arr[8] = "chdir";
  syscall_arr[9] = "dup";
  syscall_arr[10] = "getpid";
  syscall_arr[11] = "sbrk";
  syscall_arr[12] = "sleep"; 
  syscall_arr[13] = "uptime";
  syscall_arr[14] = "open";
  syscall_arr[15] = "write";
  syscall_arr[16] = "mknod";
  syscall_arr[17] = "unlink";
  syscall_arr[18] = "link";
  syscall_arr[19] = "mkdir";
  syscall_arr[20] = "close";
  syscall_arr[21] = "inc_num";
  syscall_arr[22] = "invoked_syscalls";
  syscall_arr[23] = "sort_syscalls";
  syscall_arr[24] = "get_count";
  syscall_arr[25] = "log_syscalls";
  syscall_arr[26] = "ticketlockinit";
  syscall_arr[27] = "ticketlocktest";
  syscall_arr[28] = "rwinit";
  syscall_arr[29] = "rwtest";
}


struct process_info process_details[128];
int process_details_counter = 0;

struct shared_memory shared_memories[128];
int shared_memory_counter = 0;


struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

struct ticketlock tl;
struct rwlock rwl;

int createTime = 1;
int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid+++++++++++++++++++++++++\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}


int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));


  pid = np->pid;

  // shared memory part
  np->number_of_shared_memories = 0;

  if(np->parent->number_of_shared_memories > 0 && np->parent->number_of_shared_memories < 16){
    for(i = 0; i < np->parent->number_of_shared_memories; i++){
      shm_attach(np->parent->shared_memory_ids[i]);
      np->shared_memory_ids[np->number_of_shared_memories] = np->parent->shared_memory_ids[i];
      np->number_of_shared_memories ++; 
    }
  }

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  if (strcmp(np->name, "sh") == 0 || np->pid < 2){
  }

  release(&ptable.lock);

  

  // storing new prcoess info in process_info_array
  process_details[process_details_counter].pid = pid;
  process_details[process_details_counter].counter = 0;
  process_details_counter += 1;

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;
  int i;
  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  for(i = 0; i < curproc->number_of_shared_memories; i++){
    shm_close(curproc->shared_memory_ids[i]);
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

int hasRunnable(struct proc* q[], int counter) {
  for (int i = 0; i < counter; i++){
    if (q[i]->state == RUNNABLE)
      return 1;
  }
  return 0;
}

struct proc* findProcessByPid(int pid) {
  for(struct proc *p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if (pid == p->pid)
      return p;
  }
  return 0;
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.

void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;

      swtch(&(c->scheduler), p->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
    release(&ptable.lock);
  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

void
inc_num(int n)
{
  cprintf("%d\n", n+1);
  return;
}

void
invoked_syscalls(int pid)
{
  int i, j, k;
  int found = 0;
  for (i = 0; i < process_details_counter; i++)
  {
    if(process_details[i].pid == pid) 
    {
      found = 1;
      for (j = 0; j < process_details[i].counter; j++) 
      {
        struct syscall_info* syscall_addr = &process_details[i].syscall_det[j];
        cprintf("number: %d  name: %s  d: %d ",syscall_addr->number,syscall_addr->name, syscall_addr->t.day);
        cprintf("h: %d m: %d s: %d\n", syscall_addr->t.hour, syscall_addr->t.minute, syscall_addr->t.second);
        for (k = 0; k < syscall_addr->arg_count; k++)
        {
          if (syscall_addr->args[k].type == NUMBER)
          {
            cprintf("int: %d ", syscall_addr->args[k].value);
          } else if (syscall_addr->args[k].type == POINTER)
          {
            cprintf("pointer: %p ", syscall_addr->args[k].pointer);
          } else if (syscall_addr->args[k].type == STRING)
          {
            cprintf("char*: %s ", syscall_addr->args[k].string);
          }
          cprintf("\n");
        }
      }
    }
  }
  if (found == 0) {
    cprintf("No such pid found...\n");
  }
  return;
}

void swap(struct syscall_info *xp, struct syscall_info *yp) 
{ 
    struct syscall_info temp = *xp; 
    *xp = *yp; 
    *yp = temp; 
} 
  
void bubble_sort(struct syscall_info arr[], int n) 
{ 
   int i, j; 
   for (i = 0; i < n-1; i++)       
    for (j = 0; j < n-i-1; j++)  
      if (arr[j].number <= arr[j+1].number) 
        swap(&arr[j], &arr[j+1]); 
} 


void
sort_syscalls(int pid)
{
  int i, index;
  struct process_info* proc_struct = 0;
  for(i = 0; i < process_details_counter; i++ ){
    if (process_details[i].pid == pid){
      proc_struct = &process_details[i];
      index = i;
      break;
    }
  }
  if (proc_struct == 0) {
    cprintf("No such pid found...\n");
    return;
  }
  bubble_sort(process_details[index].syscall_det, process_details[index].counter);
  return;
}

void
get_count(int pid, int num)
{
  int i, count = 0;
  struct process_info* proc_struct = 0;
  for (i = 0; i < process_details_counter; i++)
  {
    if (process_details[i].pid == pid)
    {
      proc_struct = &process_details[i];
      break;
    }    
  }
  if (proc_struct == 0)
  {
    cprintf("No such pid found...\n");
  }
  for (i = 0; i < proc_struct->counter; i++)
  {
    if (proc_struct->syscall_det[i].number == num)
      count ++;
  }
  cprintf("%d\n", count);
  return;
}

void
log_syscalls()
{
  int i;
  for (i = 0; i < sys_log_counter; i++)
  {
    cprintf("pid: %d, name: %s, ",sys_log[i]->pid, sys_log[i]->name);
    struct rtcdate time = sys_log[i]->t;
    cprintf("d:%d, h: %d, m:%d, s:%d\n", time.day, time.hour, time.minute, time.second);
  }

  return;
}

void 
ticketlockinit(void)
{
  cprintf("ticketlockinit\n");
  initticket(&tl);
  return;
}

void 
ticketlocktest(void)
{
  acquireticket(&tl);
  releaseticket(&tl); 
  return;
}

void
rwinit(void)
{
  cprintf("rwinit\n");
  initrwlock(&rwl);
  return;
}

void decimal_to_binary(uint num, int* result, int *index){
  int bin[500];

  int i = 0, k = 0;
  while(num > 0){
    bin[i] = num % 2;
    num /= 2;
    i ++;
  }
  *index = i;
  for (int j = i - 1; j >= 0; j--){
    result[k] = bin[j];
    k++;
  }
}

void 
rwtest(uint num)
{
  int bin[100];
  int index;
  decimal_to_binary(num, bin, &index);
  for (int i = 1; i < index; i++){
    pushcli();
    int pid = myproc()->pid;
    popcli();
    if (bin[i] == 0){
      cprintf("+++++++++++++++++++++++reading in loop: %d in process: %d\n", i, pid);
      fetch_and_add(&rwl.num_readers, 1);
      acquireread(&rwl);
      releaseread(&rwl);
    }
    else{
      acquirewrite(&rwl);
      releasewrite(&rwl);
      cprintf("++++++++++++++++++++++++writing in loop: %d in process: %d\n", i, pid);
    }
  }

  return;

}


void
sleepticket(void* chan)
{
  acquire(&ptable.lock);
  struct proc* p = myproc(); 
  if (p == 0)
    panic("sleep");
  p->chan = chan;
  p->state = SLEEPING;
  popcli();
  sched();
  pushcli();
  p->chan = 0;
  release(&ptable.lock);
}

void 
dealloc(void)
{
  return;
}


//enum shm_flag {NOFLAG, ONLY_OWNER_WRITE, ONLY_CHILD_CAN_ATTACH, BOTH_FLAGS};

int 
shm_open(int id, int page_count, int flag) {
  struct proc* p = myproc();
  if(p->number_of_shared_memories > 15){
    cprintf("number of shared memory for current process is overfilled \n");
    return -1;
  }
  
  p->shared_memory_ids[p->number_of_shared_memories] = id;
  p->number_of_shared_memories ++;

  for (int i = 0; i < shared_memory_counter; i++) {
    if (shared_memories[i].id == id) {
      return -1;
    }
  }
  struct shared_memory* shm = &shared_memories[shared_memory_counter++];
  shm->owner_pid = myproc()->pid;
  shm->id = id;
  shm->flag = flag;
  shm->ref_count = 0;
  shm->size = page_count;
  shm->frame_counter = 0;
  shm->is_valid = 1;
  for (int i = 0; i < page_count; i++) {
    char* new_frame = kalloc();
    if (new_frame == 0) {
      cprintf("free memory filled \n");
      return -1;
    }
    shm->frames[shm->frame_counter++] = new_frame;
  }
  return 0;
}

void* 
shm_attach(int id) {
  struct shared_memory* shm = 0;
  for (int i = 0; i < shared_memory_counter; i++) {
    if (shared_memories[i].id == id && shared_memories[i].is_valid) {
      shm = &shared_memories[i];
      break;
    }
  }
  if (shm == 0)
    return 0;


  if (shm->flag == ONLY_CHILD_CAN_ATTACH && shm->owner_pid != myproc()->parent->pid) 
    return 0;

  shm->ref_count++;

  struct proc *p;

  p = myproc();

  if(p->pid == shm->owner_pid){
    mappages(p->pgdir, (void*)p->sz, PGSIZE*shm->frame_counter, V2P(shm->frames[0]), PTE_W|PTE_U|PTE_P);
  } else if (shm->flag != ONLY_OWNER_WRITE && shm->flag != BOTH_FLAGS) {
    mappages(p->pgdir, (void*)p->sz, PGSIZE*shm->frame_counter, V2P(shm->frames[0]), PTE_W|PTE_U|PTE_P);
  } else {
    mappages(p->pgdir, (void*)p->sz, PGSIZE*shm->frame_counter, V2P(shm->frames[0]), PTE_U|PTE_P);
  }
  p->sz += PGSIZE * shm->frame_counter;

  cprintf("the frame counter is %d \n", shm->frame_counter);
  cprintf("the size is: %d \n",p->sz);

  return 0;
}

int 
shm_close(int id) {
  struct shared_memory* shm = 0;
  for (int i = 0; i < shared_memory_counter; i++) {
    if (shared_memories[i].id == id && shared_memories[i].is_valid) {
      shm = &shared_memories[i];
      break;
    }
  }
  if (shm == 0) {
    return -1;
  }
  shm->ref_count--;
  cprintf("free shared memory++ \n");
  if (shm->ref_count == 0) {
    cprintf("free shared memory \n");
    for (int i = 0; i < shm->frame_counter; i++) {
      kfree(shm->frames[i]);
    }
    shm->is_valid = 0;
  }
  return 0;
}
