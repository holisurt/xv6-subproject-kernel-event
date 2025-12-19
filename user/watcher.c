#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  struct event ev;
  uint64 first_timestamp = 0;
  
  printf("Event Watcher Started (PID: %d)\n", getpid());
  printf("Listening for kernel events...\n");
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
    
    // Print the event in a nicely formatted output
    // Format: [time_delta_ms] [pid] [event_name]
    printf("[%lu] [%d] %s\n", delta_ms, ev.pid, ev.name);
  }
  
  exit(0);
}
