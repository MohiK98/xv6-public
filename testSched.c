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
    for (int i = 1; i < NCHILD; i++){
        if (pid != 0){
            pid = fork();
        } else {
            if (i < 6) {
                setProcType(getpid(), PRIORITY);
            } else if (i < 11) {
                setProcType(getpid(), FCFS);
            }else {
                setProcType(getpid(), LOTTERY);
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