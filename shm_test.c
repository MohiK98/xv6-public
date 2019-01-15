#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define NUM_OF_CHILDS 10

int
main(int argc, char *argv[])
{
	int page_count = 4;
	int id = 10;
	int flag = 1;
	shm_open(id, page_count, flag);
	for (int i = 0; i < NUM_OF_CHILDS; i++) {
		int pid = fork();
		if (pid == 0){
			shm_attach(id);
			shm_close(id);
			return 0;
		} 
	}
	for (int i = 0; i < NUM_OF_CHILDS; i++) {
		wait();
	}
	shm_close(id);
	exit();
}
