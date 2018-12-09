#include "ticketlock.h"

struct rwlock {
	uint num_readers;
	struct ticketlock entrance_lock;
	struct ticketlock read_lock;
};	

void initrwlock(struct rwlock *rwl);
void acquireread(struct rwlock* rwl);
void releaseread(struct rwlock* rwl);
void acquirewrite(struct rwlock* rwl);
void releasewrite(struct rwlock* rwl);

