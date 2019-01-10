#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  if(argc < 2){
    printf(2, "Usage: shm_attach id\n");
    exit();
  }

  shm_attach(atoi(argv[1]));

  exit();
}
