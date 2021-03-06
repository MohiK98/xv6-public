#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "ticketlock.h"

void 
initticket(struct ticketlock *tl){
	tl->next = 0;
	tl->current = 0;
}


void 
acquireticket(struct ticketlock* tl)
{
	pushcli();
    int pid = myproc()->pid;
    popcli();
	cprintf("Acquiring ticket lock for process: %d\n", pid);
	uint ticket = fetch_and_add(&tl->next, 1);
	while(tl->current != ticket){
		sleepticket(tl);
	}
}


void
releaseticket(struct ticketlock* tl)
{
	// pushcli();
    // int pid = myproc()->pid;
	
    // popcli();
	// cprintf("Releasing lock for process:\n");
	fetch_and_add(&tl->current, 1);	
	wakeup(tl);
}