#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "rwlock.h"

void acquireread(struct rwlock* rwl){
	cprintf("r\n");
	acquireticket(&rwl->entrance_lock);
	uint num_readers = fetch_and_add(&rwl->num_readers, 1);
	if (num_readers == 1){
		acquireticket(&rwl->read_lock);
	}
	releaseticket(&rwl->entrance_lock);
}

void releaseread(struct rwlock* rwl){
	acquireticket(&rwl->entrance_lock);
	uint num_readers = fetch_and_add(&rwl->num_readers, -1);
	if (num_readers == 0){
		releaseticket(&rwl->read_lock);
	}
	releaseticket(&rwl->entrance_lock);
}

void acquirewrite(struct rwlock* rwl){
	cprintf("w\n");
	acquireticket(&rwl->read_lock);
}

void releasewrite(struct rwlock* rwl){
	releaseticket(&rwl->read_lock);
}