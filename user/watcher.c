#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

// Simple utility to write integer to file descriptor
void
write_int(int fd, int val)
{
  char buf[32];
  int len = 0;
  int n = val;
  
  if(n == 0) {
    write(fd, "0", 1);
    return;
  }
  
  if(n < 0) {
    write(fd, "-", 1);
    n = -n;
  }
  
  while(n > 0) {
    buf[len++] = '0' + (n % 10);
    n /= 10;
  }
  
  for(int i = len - 1; i >= 0; i--) {
    write(fd, &buf[i], 1);
  }
}

int
main(int argc, char *argv[])
{
  struct event ev;
  uint64 first_timestamp = 0;
  int filter_mode = 0;  // 0 = all events, 1 = important only (fork, sleep)
  int outfd = 1;  // Default to stdout
  
  // Parse arguments: -i for important only, -o filename for output file
  for(int i = 1; i < argc; i++) {
    if(argv[i][0] == '-') {
      if(argv[i][1] == 'i' || argv[i][1] == 'I') {
        filter_mode = 1;
      } else if(argv[i][1] == 'o' || argv[i][1] == 'O') {
        if(i + 1 < argc) {
          outfd = open(argv[i+1], O_CREATE | O_WRONLY);
          if(outfd < 0) {
            printf("watcher: could not open %s\n", argv[i+1]);
            exit(1);
          }
          i++;
        }
      }
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
        write(outfd, "[", 1);
        write_int(outfd, delta_ms);
        write(outfd, "] [", 3);
        write_int(outfd, ev.pid);
        write(outfd, "] ", 2);
        write(outfd, ev.name, strlen(ev.name));
        write(outfd, "\n", 1);
        // Larger delay to prevent output overlap with shell commands
        int start = uptime();
        while(uptime() - start < 10) {  // Wait ~10 ticks (~100ms)
          // busy spin
        }
      } else if(ev.name[0] == 's') {  // sleep
        write(outfd, "[", 1);
        write_int(outfd, delta_ms);
        write(outfd, "] [", 3);
        write_int(outfd, ev.pid);
        write(outfd, "] ", 2);
        write(outfd, ev.name, strlen(ev.name));
        write(outfd, "\n", 1);
        // Larger delay to prevent output overlap with shell commands
        int start = uptime();
        while(uptime() - start < 10) {  // Wait ~10 ticks (~100ms)
          // busy spin
        }
      }
      // Skip write events and others
    } else {
      // Show all events
      printf("[%lu] [%d] %s\n", delta_ms, ev.pid, ev.name);
    }
  }
  
  exit(0);
}
