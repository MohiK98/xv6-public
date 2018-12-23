#include "types.h"
#include "user.h"

#define NCHILD 15
#define LOTTERY 0
#define FCFS 1
#define PRIORITY 2


int main(){
	int pid = fork();
    if (pid == 0){
        setProcType(getpid(), FCFS);
    }
    setProcType(2, FCFS);
    for (int i = 1; i < NCHILD; i++){
        if (pid != 0){
            pid = fork();
        } else {
            pti();
            if (i < 6) {
                setProcType(getpid(), LOTTERY);
                setLotteryTicketRange(getpid(), i*4);
            } else if (i < 11) {
                setProcType(getpid(), FCFS);
                for(int i = 0; i < 1000; i++);
            }else {
                setProcType(getpid(), PRIORITY);
                for(int i = 0; i < 1000; i++);
            }
        }
    }
    if (pid != 0) {
        for (int i = 0; i < NCHILD; i++){
            pti();
            wait();
        }
        printf(1, "User Program finished.\n");
    }
    exit();
}