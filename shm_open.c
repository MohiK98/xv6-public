#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  if(argc < 4){
    printf(2, "Usage: shm_open id page_count flag\n");
    exit();
  }
  
  shm_open(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));

  exit();
}
