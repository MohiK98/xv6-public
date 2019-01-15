#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define NUM_OF_CHILDS 10

int
main(int argc, char *argv[])
{

	if (argc != 4){
		printf(1, "usage: shared memory id, pageCount, flag\n");
		return 0;
	}
	int page_count = atoi(argv[2]);
	int id = atoi(argv[1]);
	int flag = atoi(argv[3]);
	shm_open(id, page_count, flag);
	shm_attach(id);
	for (int i = 0; i < NUM_OF_CHILDS; i++) {
		int pid = fork();
		if (pid == 0){
			shm_attach(id);
			// shm_close(id);
			exit();
		} 
	}
	for (int i = 0; i < NUM_OF_CHILDS; i++) {
		wait();
	}
	// shm_close(id);
	exit();
}
