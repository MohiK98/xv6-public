#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "rwlock.h"

void initrwlock(struct rwlock *rwl){
	rwl->num_readers = 0;
	initticket(&rwl->entrance_lock);	
	initticket(&rwl->read_lock);	
}

void acquireread(struct rwlock* rwl){
	// cprintf("r\n");
	cprintf("++++++++++++++++++num_readers is: %d\n", rwl->num_readers);
	acquireticket(&rwl->entrance_lock);
	// pushcli();
	// rwl->num_readers += 1;
	// uint num_readers = fetch_and_add(&rwl->num_readers, 1);
	if (rwl->num_readers == 1){
		// popcli();	
		// cprintf("process with pid: %d in num_readers: ", myproc()->pid);
		acquireticket(&rwl->read_lock);
		// pushcli();
	}
	// popcli();
	releaseticket(&rwl->entrance_lock);
}

void releaseread(struct rwlock* rwl){
	acquireticket(&rwl->entrance_lock);
	// pushcli();
	// rwl->num_readers--;
	uint num_readers = rwl->num_readers;
	if (num_readers == 0){
		// popcli();
		releaseticket(&rwl->read_lock);
		// pushcli();
	}
	// popcli();
	releaseticket(&rwl->entrance_lock);
}

void acquirewrite(struct rwlock* rwl){
	// cprintf("w\n");
	acquireticket(&rwl->read_lock);
}

void releasewrite(struct rwlock* rwl){
	releaseticket(&rwl->read_lock);
}