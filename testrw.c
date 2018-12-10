#include "types.h"
#include "user.h"

#define NCHILD 10

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
		rwtest(15);
		printf(1, "child adding to rw\n");
	}
	else {
		for (int i = 0; i < NCHILD + 1; i++)
			wait();
		printf(1, "user program finished\n");
	}
	exit();
}