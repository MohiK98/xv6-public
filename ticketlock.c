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
	cprintf("a:t:%d\n", tl->next);
	uint ticket = fetch_and_add(&tl->next, 1);
	cprintf("a:t:%d\n", tl->next);
	pushcli();
	cprintf("Acquiring ticket lock for process: %d with ticket: %d\n", myproc()->pid, ticket);
	while(tl->current != ticket){
		sleepticket(tl);
	}
	popcli();
}


void
releaseticket(struct ticketlock* tl)
{
	// cprintf("Releasing lock for process: %d\n", myproc()->pid);
	cprintf("current: %d\n", tl->current );
	fetch_and_add(&tl->current, 1);
	wakeup(tl);
}