#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  if(argc < 2){
    printf(2, "Usage: invoked_syscalls pid\n");
    exit();
  }

  // printf(1, "asdf\n");
  invoked_syscalls(atoi(argv[1]));

  exit();
}
