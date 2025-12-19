#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  struct event ev;
  uint64 first_timestamp = 0;
  int filter_mode = 0;  // 0 = all events, 1 = important only (fork, sleep)
  
  // Check if user passed argument for filtering
  if(argc > 1 && argv[1][0] == '-') {
    if(argv[1][1] == 'i' || argv[1][1] == 'I') {
      filter_mode = 1;  // Important events only
    }
  }
  
  printf("Event Watcher Started (PID: %d)\n", getpid());
  printf("Listening for kernel events...\n");
  if(filter_mode == 1) {
    printf("Filter: Important events only (fork, sleep)\n");
  } else {
    printf("Filter: All events (use -i for important only)\n");
  }
  printf("====================================\n");
  printf("Format: [Delta(ms)] [PID] [Event]\n");
  printf("====================================\n");
  
  while(1) {
    // Wait for an event from kernel queue (blocking)
    if(kqueue_wait(&ev) < 0) {
      printf("Error waiting for event\n");
      break;
    }
    
    // Set baseline timestamp from first event
    if(first_timestamp == 0) {
      first_timestamp = ev.timestamp;
    }
    
    // Calculate time delta from first event (in milliseconds)
    uint64 delta_us = (ev.timestamp - first_timestamp);
    uint64 delta_ms = delta_us / 1000;
    
    // Filter events if in important-only mode
    if(filter_mode == 1) {
      // Only show fork and sleep events
      if(ev.name[0] == 'f') {  // fork
        printf("[%lu] [%d] %s\n", delta_ms, ev.pid, ev.name);
      } else if(ev.name[0] == 's') {  // sleep
        printf("[%lu] [%d] %s\n", delta_ms, ev.pid, ev.name);
      }
      // Skip write events and others
    } else {
      // Show all events
      printf("[%lu] [%d] %s\n", delta_ms, ev.pid, ev.name);
    }
  }
  
  exit(0);
}
