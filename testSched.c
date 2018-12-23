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
            counter ++;
        }
        if (pid == 0){
            if (counter < 6){
                setProcType(getpid(), PRIORITY);
                printf(1, "oh come on babe\n");
            }
            else if (counter < 11){
                printf(1, "some kind of shit\n");
                setProcType(getpid(), FCFS);
            } 
            else{
                printf(1, "yes suck my dick\n");
                setProcType(getpid(), LOTTERY);
                setLotteryTicketRange(getpid(), counter*3);
            }
        }
    }

	if (pid < 0)
		printf(2, "fork error\n");
	// else if (pid == 0){
    //     if (counter < 6){
    //         setProcType(getpid(), PRIORITY);
    //         printf(1, "oh come on babe\n");
    //     }
    //     else if (counter < 11){
    //         printf(1, "some kind of shit\n");
    //         setProcType(getpid(), FCFS);
    //     } 
    //     else{
    //         printf(1, "yes suck my dick\n");
    //         setProcType(getpid(), LOTTERY);
    //         setLotteryTicketRange(getpid(), counter*3);
    //     }
    // }
	else {
        setProcType(getpid(), PRIORITY);
    	for (int i = 0; i < NCHILD + 1; i++){
            pti();
            wait();
        }
		printf(1, "user program finished\n");
    }
    exit();

}