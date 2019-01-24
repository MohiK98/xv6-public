#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define NUM_OF_CHILDS 1 

int
main(int argc, char *argv[])
{

	char* arr;
	if (argc != 4){
		printf(1, "usage: shared memory id, pageCount, flag\n");
		exit();
	}
	int page_count = atoi(argv[2]);
	int id = atoi(argv[1]);
	int flag = atoi(argv[3]);
	shm_open(id, page_count, flag);
	
	for (int i = 0; i < NUM_OF_CHILDS; i++) {
		int pid = fork();
		if (pid == 0){
			arr = shm_attach(id);
			arr[5] = 'a';
			printf(2,"______________ %c\n", arr[5]);
			shm_close(id);
			exit();
		} 
	}
	for (int i = 0; i < NUM_OF_CHILDS; i++) {
		wait();
	}
	arr = shm_attach(id);
	printf(2, "++++++ %c \n", arr[5]);
	exit();
}
