#include <types.h>
#include <stat.h>
#include <user.h>

int getd(char* buf){ // parse char into int 
    int ret = 0;
    while (*buf) { 
        ret = ret*10 + *buf - '0';
        ++buf;
    }
    return ret;
}

int
main(int argc, char *argv[])
{
  int i,j;

  if(argc != 3){
    printf(1, "Usage, loop n prio\nLoop for n^2 times with prio priority(priority is between 1 and 100)\n");
    exit();
  }

  int n = getd(argv[1]);
  int prio = getd(argv[2]);
  setprio(prio);
  for(i=0;i<n;i++)
  for(j=0;j<n;j++);
  exit();
}
