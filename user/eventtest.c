#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("Test: Creating fork events\n");
  
  // Create a few child processes
  for(int i = 0; i < 3; i++) {
    int pid = fork();
    if(pid == 0) {
      // Child process
      printf("Child %d running\n", getpid());
      exit(0);
    } else if(pid > 0) {
      // Parent waits for child
      wait(0);
    } else {
      printf("fork failed\n");
    }
  }
  
  printf("Test: Writing to console\n");
  write(1, "Hello from test\n", 16);
  
  exit(0);
}
