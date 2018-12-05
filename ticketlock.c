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
	acquire(&tl->lk);
	tl->locked = 0;
	tl->current++;
	wakeup(tl);
	release(&tl->lk);
}

// void
// sleepticket(void* chan, struct spinlock* tl)
// {
// 	struct proc *p = myproc();
// 	if(p == 0)
// 		panic("sleep");
// 	if(tl == 0)
// 		panic("sleep without lk");
// 	if(tl != &ptable.lock){
// 		acquire(&ptable.lock);
// 		release(tl);
// 	}
// 	// Go to sleep.
// 	p->chan = chan;
// 	p->state = SLEEPING;
// 	sched();
// 	// Tidy up.
// 	p->chan = 0;
// 	// Reacquire original lock.
// 	if(tl != &ptable.lock){
// 		release(&ptable.lock);
// 		acquire(tl);
// 	}
// }

void 
acquireticket(struct ticketlock* tl)
{
	acquire(&tl->lk);
	asm("inc %0": "+r"(tl->next));
	uint ticket = tl->next;
	while(tl->current != ticket){
		sleep(tl, &tl->lk);
	}
	tl->locked = 1;
	release(&tl->lk);

}