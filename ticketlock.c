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
acquireticket(struct ticketlock* tl)
{
	cprintf("Acquiring lock for process: %d\n", myproc()->pid);
	pushcli();
	tl->locked = 1;
	uint ticket = tl->next;

	cprintf("tl->next: %d\n", tl->next);
	asm("inc %0": "+r"(tl->next));
	cprintf("tl->next: %d\n", tl->next);
	while(tl->current != ticket){
		sleepticket(tl);
	}
	popcli();
}


void
releaseticket(struct ticketlock* tl)
{
	pushcli();
	tl->locked = 0;
	tl->current++;
	wakeup(tl);
	popcli();
}