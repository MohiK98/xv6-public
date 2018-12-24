#include "types.h"
#include "user.h"

int main(int argc , char *argv[])
{
	if(argc < 3){
	    printf(2, "Usage: pid newType\n");
	    exit();
  	}
    setProcType(atoi(argv[1]), atoi(argv[2]));
    exit();
}