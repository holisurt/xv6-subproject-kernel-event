# Understanding Kernel Events: fork, sleep, write

## Overview

The Kernel Event Queue System captures three types of system events that represent the core behavior of processes:

1. **fork** - Process creation
2. **sleep** - Process waiting/blocking
3. **write** - Data output

This document explains each event type in detail with examples and use cases.

---

## 1. FORK Event - Process Creation

### What is fork?

Fork is a **system call that creates a new process** (called the child process) that is an exact copy of the current process (called the parent process).

**System Call:**
```c
int pid = fork();
```

**Returns:**
- In parent: returns the child's process ID (positive number)
- In child: returns 0
- On error: returns -1

### Code Example

```c
#include "user/user.h"

int main() {
  int pid = fork();
  
  if(pid == 0) {
    // This is the CHILD process
    printf("I am the child process\n");
    exit(0);
  } else if(pid > 0) {
    // This is the PARENT process
    printf("I am the parent, created child with PID %d\n", pid);
    wait(0);  // Wait for child to finish
  } else {
    // Error
    printf("fork failed\n");
  }
  
  return 0;
}
```

### When is fork event triggered?

The kernel records a **fork event** at the moment `fork()` is successfully called.

**Kernel side (in your implementation):**
- Located in `kernel/proc.c` in the `fork()` function
- Calls `kqueue_post()` to record the fork event

### Real-World Example in QEMU

**Running eventtest:**
```
$ eventtest
Test: Creating fork events
Child 7 running
Child 8 running
Child 9 running
Test: Writing to console
Hello from test
```

**Events captured:**
```
[31] [2] fork      # eventtest (PID 2) creates child 1
[221955] [8] fork  # eventtest (PID 8) creates child 2
[...]
```

**Why it happens:** eventtest has a loop that calls fork() 3 times.

### Real-World Use Cases

| Scenario | What fork creates |
|----------|------------------|
| `$ ls` | Shell forks, creates ls process |
| `$ cat file.txt` | Shell forks, creates cat process |
| `$ background_job &` | Shell forks, creates background process |
| System startup | init forks many processes |
| Web server | Server forks worker processes |

### Why is fork important?

- ✅ Shows when new processes are created
- ✅ Helps debug process creation problems
- ✅ Track parent-child process relationships
- ✅ Understand process tree structure
- ✅ Identify process spawning bottlenecks

---

## 2. SLEEP Event - Process Waiting/Blocking

### What is sleep?

Sleep makes a **process pause and wait** for:
- A specified time period to elapse, OR
- I/O operation to complete (disk read, keyboard input, network data), OR
- A condition to be met

**System Call:**
```c
int sleep(int ticks);  // Wait for 'ticks' timer ticks
```

### Code Example

```c
#include "user/user.h"

int main() {
  printf("Starting long operation...\n");
  
  // Wait for 2 seconds
  sleep(2);
  
  printf("Finished waiting\n");
  return 0;
}
```

### When is sleep event triggered?

The kernel records a **sleep event** when:

1. **Explicit sleep call:**
   ```c
   sleep(5);  // Sleep for 5 ticks
   ```

2. **I/O waiting:**
   ```c
   read(fd, buf, 100);  // Wait for disk/input
   ```

3. **Process synchronization:**
   ```c
   wait(0);  // Parent waits for child
   ```

### Real-World Example - Reading a File

**User runs:**
```
$ cat myfile.txt
Hello World
```

**What happens internally:**
```
Time    Event    Description
----    -----    -----------
[0]     fork     Shell creates cat process
[10]    write    cat calls open()
[20]    sleep    cat calls read() - waits for disk!
[100]   write    Disk returns data, cat prints it
```

The **sleep event at [20]** represents the time cat is blocked waiting for disk I/O.

### Real-World Use Cases

| Scenario | Why process sleeps |
|----------|-------------------|
| Reading file from disk | Waiting for disk I/O |
| Waiting for user input | Waiting for keyboard |
| Network request | Waiting for network data |
| `sleep(5)` command | Explicit timer wait |
| Parent `wait()` | Waiting for child process |
| Pipe communication | Waiting for data from other process |

### Why is sleep important?

- ✅ Shows when processes are blocked/waiting
- ✅ Identifies I/O bottlenecks
- ✅ Find performance issues
- ✅ Detect if process is stuck
- ✅ Track resource contention

---

## 3. WRITE Event - Data Output

### What is write?

Write **sends data to a destination** (console, file, pipe, network, etc.) through a **file descriptor**.

**System Call:**
```c
int write(int fd, const void *buf, int n);
```

**Parameters:**
- `fd` - File descriptor (1=stdout/console, 2=stderr, 3+=files)
- `buf` - Data to write
- `n` - Number of bytes

**Returns:** Number of bytes written

### Code Example

```c
#include "user/user.h"

int main() {
  // Direct write to stdout (fd=1)
  write(1, "Hello World\n", 12);
  
  // printf() internally uses write()
  printf("This also uses write()\n");
  
  return 0;
}
```

### When is write event triggered?

The kernel records a **write event** when:

1. **Console output:**
   ```c
   printf("Hello\n");    // Uses write internally
   write(1, "Hello\n", 6);
   ```

2. **File I/O:**
   ```c
   write(fd, data, len);  // Write to file
   ```

3. **Pipe communication:**
   ```c
   write(pipefd[1], msg, len);  // Send data to another process
   ```

### Real-World Example - Echo Command

**User runs:**
```
$ echo hello
hello
```

**Events captured:**
```
[0]     fork    Shell creates echo process
[10]    write   echo outputs "hello" to console
```

**Timeline:**
1. Shell calls `fork()` → **fork event**
2. Child process (echo) runs
3. echo calls `printf("hello\n")` which internally calls `write()` → **write event**
4. Console displays: `hello`

### Real-World Use Cases

| Scenario | What gets written |
|----------|------------------|
| `$ echo text` | Text output to console |
| `$ ls` | Directory listing to console |
| `$ cat file` | File contents to console |
| File creation | Data written to disk |
| Pipe `\|` | Data sent through pipe |
| Log file | Debug info written to file |
| Network socket | Data sent over network |

### Why is write important?

- ✅ Shows process output activity
- ✅ Track what data is being produced
- ✅ Identify excessive output (performance issue)
- ✅ Debug why output is missing
- ✅ Monitor I/O patterns

---

## Visual Timeline Example

When you run `eventtest` in QEMU, here's the complete timeline:

```
Time(ms)    PID    Event    Description
---------   ---    -----    -----------
[0]         2      fork     eventtest forks, creates child 1
[5]         7      write    Child 1 prints "Child 7 running"
[10]        7      sleep    Child 1 calls exit(), waits for cleanup
[20]        2      fork     eventtest forks, creates child 2
[25]        8      write    Child 2 prints "Child 8 running"
[30]        8      sleep    Child 2 exits
[40]        2      fork     eventtest forks, creates child 3
[45]        9      write    Child 3 prints "Child 9 running"
[50]        9      sleep    Child 3 exits
[60]        2      write    eventtest prints "Test: Writing to console"
[65]        2      write    eventtest prints "Hello from test"
```

### Interpretation

**Fork events [0, 20, 40]:**
- eventtest creates 3 child processes sequentially
- Each fork happens ~20ms apart (parent waits for child to exit before next fork)

**Write events [5, 25, 45, 60, 65]:**
- Each child prints its PID when created
- Parent prints header and message at end

**Sleep events (filtered out with `-i` flag):**
- Children sleeping during exit cleanup
- Not shown because we use `-i` (important only) flag

---

## How to Capture These Events

### Basic Usage

```bash
$ watcher
# Prints all events to console
```

### Important Events Only (no write spam)

```bash
$ watcher -i &
# Shows only fork and sleep events
# Runs in background (&)
```

### Log to File (cleanest output)

```bash
$ watcher -i -o events.log &
# Important events only, logged to file
# Console stays clean
# Review with: cat events.log
```

### Complete Test Session

```bash
$ rm events.log
$ watcher -i -o events.log &
$ sleep 1                      # Let watcher start
$ eventtest                    # Creates fork events
$ echo "Hello"                 # Fork + write
$ cat events.log               # View captured events
```

### Expected Output

```
[0] [2] fork
[50] [7] fork
[100] [8] fork
[150] [9] fork
[200] [2] write
[250] [7] sleep
[300] [8] sleep
[350] [9] sleep
```

---

## Implementation Details

### Where Events are Recorded

**fork event:**
- File: `kernel/proc.c`
- Function: `fork()`
- Line: Calls `kqueue_post(p->pid, "fork")`

**sleep event:**
- File: `kernel/proc.c`
- Function: `sleep()`
- Line: Calls `kqueue_post(p->pid, "sleep")`

**write event:**
- File: `kernel/sysfile.c`
- Function: `sys_write()`
- Line: Calls `kqueue_post(p->pid, "write")`

### Event Structure

```c
struct event {
  int pid;                    // Process ID (4 bytes)
  char name[32];             // Event name: "fork", "sleep", "write" (32 bytes)
  uint64 timestamp;          // Kernel clock ticks when event occurred (8 bytes)
};                          // Total: 44 bytes
```

### Circular Buffer

Events are stored in a circular buffer in kernel memory:

```c
#define MAXEVENTQUEUE 256

struct {
  struct event queue[MAXEVENTQUEUE];
  int front;
  int rear;
  spinlock_t lock;
} kqueue;
```

- Holds up to 256 events
- When full, oldest events are overwritten
- Thread-safe with spinlock

---

## Summary Table

| Event | When | Why | Example |
|-------|------|-----|---------|
| **fork** | New process created | Process creation tracking | `eventtest` creates 3 children |
| **sleep** | Process waits | Identify bottlenecks | Reading from disk |
| **write** | Data output | Track process activity | `echo hello` outputs text |

---

## Common Questions

### Q: Why do I see so many sleep events?

**A:** The watcher itself uses sleep to prevent output overlap when running in background. Use `watcher -i -o file.log &` to log to file instead - this avoids capturing watcher's own events.

### Q: What's the difference between sleep and write?

**A:**
- **sleep**: Process is WAITING (idle, blocked, no CPU)
- **write**: Process is PRODUCING OUTPUT (active, has data)

### Q: Can I write my own events?

**A:** Yes! Use the syscall:
```c
kqueue_post(getpid(), "my_event");
```

### Q: Are events lost if buffer is full?

**A:** Yes, oldest events are overwritten. The circular buffer holds 256 events max. For continuous monitoring, regularly read with watcher.

### Q: How accurate are timestamps?

**A:** Timestamps are in kernel ticks (measured in 10ms increments on RISC-V). Accurate to ~10ms resolution.

---

## Next Steps

1. Run `make qemu` to start the system
2. Use `watcher -i -o events.log &` to monitor events
3. Run commands: `eventtest`, `forktest`, `echo`, `ls`
4. Review events: `cat events.log`
5. Study the event patterns to understand process behavior

For more information, see `USAGE_GUIDE.md` and `DESIGN_DOCUMENT.md`.
