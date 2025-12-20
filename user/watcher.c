#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAX_EVENTS 1000

// Event buffer to store all events
struct {
  struct event ev;
  uint64 delta_ms;
} event_buffer[MAX_EVENTS];

int event_count = 0;

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

// Write formatted event to file descriptor
void
write_event(int fd, uint64 delta_ms, int pid, const char *name)
{
  write(fd, "[", 1);
  write_int(fd, delta_ms);
  write(fd, "] [", 3);
  write_int(fd, pid);
  write(fd, "] ", 2);
  write(fd, (char *)name, strlen(name));
  write(fd, "\n", 1);
}

int
main(int argc, char *argv[])
{
  struct event ev;
  uint64 first_timestamp = 0;
  int filter_mode = 0;  // 0 = fork only, 1 = important (fork/sleep), 2 = contextual (forks with nearby sleep/write)
  int outfd = 1;  // Default to stdout
  int watcher_pid = getpid();  // Store our own PID to filter out our events
  int context_window = 10000;  // Default: 10 seconds in milliseconds
  
  // Parse arguments
  for(int i = 1; i < argc; i++) {
    if(argv[i][0] == '-') {
      if(argv[i][1] == 'i' || argv[i][1] == 'I') {
        filter_mode = 1;  // Important events (fork, sleep)
      } else if(argv[i][1] == 'c' || argv[i][1] == 'C') {
        filter_mode = 2;  // Contextual mode: show forks with nearby sleep/write
        if(i + 1 < argc && argv[i+1][0] != '-') {
          context_window = atoi(argv[i+1]) * 1000;  // Convert seconds to ms
          i++;
        }
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
  
  // Only print startup messages to stdout
  if(outfd == 1) {
    printf("Event Watcher Started (PID: %d)\n", getpid());
    printf("Listening for kernel events...\n");
    if(filter_mode == 0) {
      printf("Mode: Fork events only\n");
    } else if(filter_mode == 1) {
      printf("Mode: Important events (fork, sleep)\n");
    } else {
      printf("Mode: Contextual (forks with related events within %d seconds)\n", context_window / 1000);
    }
    printf("====================================\n");
    printf("Format: [Delta(ms)] [PID] [Event]\n");
    printf("====================================\n");
  }
  
  if(filter_mode != 2) {
    // Original mode: stream events as they come
    while(1) {
      if(kqueue_wait(&ev) < 0) {
        if(outfd == 1) {
          printf("Error waiting for event\n");
        }
        break;
      }
      
      // Skip events from watcher itself
      if(ev.pid == watcher_pid) {
        continue;
      }
      
      // Set baseline timestamp from first event
      if(first_timestamp == 0) {
        first_timestamp = ev.timestamp;
      }
      
      uint64 delta_us = (ev.timestamp - first_timestamp);
      uint64 delta_ms = delta_us / 1000;
      
      // Filter events
      if(filter_mode == 0) {
        // Fork events only
        if(ev.name[0] == 'f') {
          write_event(outfd, delta_ms, ev.pid, ev.name);
        }
      } else if(filter_mode == 1) {
        // Fork and sleep events
        if(ev.name[0] == 'f' || ev.name[0] == 's') {
          write_event(outfd, delta_ms, ev.pid, ev.name);
        }
      }
    }
  } else {
    // Contextual mode: buffer events and print forks with context
    while(1) {
      if(kqueue_wait(&ev) < 0) {
        if(outfd == 1) {
          printf("Error waiting for event\n");
        }
        break;
      }
      
      // Skip events from watcher itself
      if(ev.pid == watcher_pid) {
        continue;
      }
      
      // Set baseline timestamp from first event
      if(first_timestamp == 0) {
        first_timestamp = ev.timestamp;
      }
      
      // Buffer the event
      if(event_count < MAX_EVENTS) {
        event_buffer[event_count].ev = ev;
        event_buffer[event_count].delta_ms = (ev.timestamp - first_timestamp) / 1000;
        
        // If this is a fork event, print it with context
        if(ev.name[0] == 'f') {
          // Find related events before and after this fork
          int idx = event_count;
          uint64 fork_time = event_buffer[idx].delta_ms;
          
          // Print recent events before this fork (within context window)
          for(int i = idx - 1; i >= 0 && i >= idx - 50; i--) {
            if(fork_time - event_buffer[i].delta_ms <= context_window) {
              if(event_buffer[i].ev.name[0] == 's' || event_buffer[i].ev.name[0] == 'w') {
                write_event(outfd, event_buffer[i].delta_ms, event_buffer[i].ev.pid, event_buffer[i].ev.name);
              }
            }
          }
          
          // Print the fork event
          write_event(outfd, fork_time, event_buffer[idx].ev.pid, event_buffer[idx].ev.name);
          write(outfd, "  ---\n", 6);  // Separator
        }
        
        event_count++;
      }
    }
  }
  
  exit(0);
}
