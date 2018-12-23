#include "types.h"
#include "user.h"

#define NCHILD 15
#define LOTTERY 0
#define FCFS 1
#define PRIORITY 2


int main(){
	int pid;
    int counter = 1;
    pid = fork();

	for (int i = 1; i < NCHILD; i++){
		if (pid > 0){
			pid = fork();
		}
        counter ++;
	}

	if (pid < 0)
		printf(2, "fork error\n");
	else if (pid == 0){
        if (counter < 6){
            setProcType(getpid(), PRIORITY);
        }
        else if (counter < 11){
            setProcType(getpid(), FCFS);
        } 
        else{
            setProcType(getpid(), LOTTERY);
            setLotteryTicketRange(getpid(), counter*3);
        }
    }
	else {
        pti();
    	for (int i = 0; i < NCHILD; i++){
			pti();
            wait();
        }
		printf(1, "user program finished\n");
        pti();
    }
	exit();
}