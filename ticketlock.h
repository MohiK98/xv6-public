//‌ Ticketlock for fairness in acquiring a lock

struct ticketlock {
	uint locked;
	uint current, next;
	struct spinlock lk; // for protecting this lock
};

void acquireticket(struct ticketlock* tl);
void releaseticket(struct ticketlock* tl);
void sleepticket(void* chan, struct spinlock* tl);

