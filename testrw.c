#include "types.h"
#include "user.h"

#define NCHILD 3

int main(){
	int pid;

	rwinit();

	pid = fork();

	for (int i = 1; i < NCHILD; i++){
		if (pid > 0){
			pid = fork();
		}
	}
	if (pid < 0)
		printf(2, "fork error\n");
	else if (pid == 0){
		printf(1, "child adding to rw\n");
		rwtest(25);
	}
	else {
		for (int i = 0; i < NCHILD; i++)
			wait();
		printf(1, "user program finished\n");
	}
	dealloc();
	exit();
}