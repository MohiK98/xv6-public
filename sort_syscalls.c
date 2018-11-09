#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  if(argc < 2){
    printf(2, "Usage: sort_syscalls pid\n");
    exit();
  }
  sort_syscalls(atoi(argv[1]));

  exit();
}
