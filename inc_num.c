#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  if(argc < 2){
    printf(2, "Usage: inc_num number\n");
    exit();
  }

  inc_num(atoi(argv[1]));

  exit();
}
