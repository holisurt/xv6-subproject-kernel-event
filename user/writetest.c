#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("=== Write Test Program ===\n");
  printf("This test demonstrates write system calls being captured by watcher\n\n");
  
  // Test 1: Multiple writes from parent
  printf("Test 1: Parent process writes\n");
  for(int i = 0; i < 3; i++) {
    printf("  Write #%d from parent\n", i + 1);
  }
  
  // Test 2: Fork and both parent and child write
  printf("\nTest 2: Parent and child both write\n");
  int pid = fork();
  
  if(pid == 0) {
    // Child process
    for(int i = 0; i < 3; i++) {
      write(1, "    [Child] Writing to console\n", 31);
    }
    exit(0);
  } else if(pid > 0) {
    // Parent waits for child
    wait(0);
    printf("Parent: child finished\n");
  }
  
  // Test 3: Multiple children writing
  printf("\nTest 3: Multiple children writing\n");
  for(int i = 0; i < 2; i++) {
    int pid2 = fork();
    
    if(pid2 == 0) {
      // Child
      for(int j = 0; j < 2; j++) {
        write(1, "    [Child] Message\n", 20);
      }
      exit(0);
    } else if(pid2 > 0) {
      // Parent continues, will wait later
    }
  }
  
  // Wait for all children
  wait(0);
  wait(0);
  
  printf("\n=== Write Test Complete ===\n");
  exit(0);
}
