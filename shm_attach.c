
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
  void* res = shm_attach(atoi(argv[1]));
  printf(2, "%s", res);

  exit();
}
