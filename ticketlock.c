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
releaseticket(struct ticketlock* tl)
{
	pushcli();
	tl->locked = 0;
	tl->current++;
	wakeup(tl);
	popcli();
}

void 
acquireticket(struct ticketlock* tl)
{
	pushcli();
	asm("inc %0": "+r"(tl->next));
	uint ticket = tl->next;

	while(tl->current != ticket){
		sleepticket(tl);
	}
	tl->locked = 1;
	popcli();
}