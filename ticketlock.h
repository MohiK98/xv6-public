//‌ Ticketlock for fairness in acquiring a lock
#ifndef TICKETLOCK_H
#define TICKETLOCK_H

struct ticketlock {
	uint current, next;
};

void initticket(struct ticketlock *tl); 
void acquireticket(struct ticketlock* tl);
void releaseticket(struct ticketlock* tl);

#endif
