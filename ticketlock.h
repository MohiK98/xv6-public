//‌ Ticketlock for fairness in acquiring a lock

struct ticketlock {
	uint current, next;
};

void acquireticket(struct ticketlock* tl);
void releaseticket(struct ticketlock* tl);

