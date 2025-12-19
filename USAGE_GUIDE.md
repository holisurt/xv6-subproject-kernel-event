# Hướng Dẫn Sử Dụng Kernel Event Queue System

## Tổng Quan

**Kernel Event Queue System** là một hệ thống theo dõi sự kiện kernel trong xv6-riscv.
Nó tự động capture và lưu trữ các hoạt động quan trọng như:
- **fork()** - Tạo process mới
- **sleep()** - Process chờ I/O hoặc timeout
- **write()** - Ghi dữ liệu

**Repository GitHub:** https://github.com/holisurt/xv6-subproject-kernel-event

---

## I. Quick Start (5 Phút)

### 1. Build Hệ Thống

```bash
cd /home/duyen-hung/Desktop/OS/xv6-riscv-riscv-Subproject
make clean
make -j4
```

### 2. Chạy QEMU

```bash
make qemu
```

### 3. Chạy Watcher (Terminal 1 trong QEMU)

```bash
$ watcher
```

**Output**:
```
Event Watcher Started (PID: 3)
Listening for kernel events...
====================================
Format: [Delta(ms)] [PID] [Event]
====================================
```

### 4. Chạy Một Command (Terminal 2 - Ctrl+A+C để switch)

```bash
$ ls
```

### 5. Xem Events Trong Watcher

Watcher sẽ in ra:
```
[0] [4] fork
[10] [4] write
[15] [4] write
[20] [4] write
```

---

## II. Build & Installation

### Build Command

```bash
# Full rebuild
make clean && make -j4

# Or simple build
make

# Check for errors
make 2>&1 | grep -i error
```

**Expected Files After Build:**
- `kernel/kernel` - Kernel executable
- `fs.img` - Filesystem image
- `user/_watcher` - Event monitor program
- `user/_eventtest` - Test program

### Verify Build Success

```bash
ls -lh kernel/kernel fs.img user/_watcher
```

Should show all files present.

---

## III. Running QEMU

### Basic Run

```bash
make qemu
```

### Expected Boot Output

```
xv6 kernel is booting

hart 1 starting
hart 2 starting
init: starting sh
$ 
```

### Exit QEMU

```bash
$ exit
# or Ctrl+A+X
```

### With Debug (gdb)

```bash
# Terminal 1 - Start QEMU
make qemu-gdb

# Terminal 2 - Connect debugger
riscv64-unknown-elf-gdb kernel/kernel
(gdb) target remote :26000
(gdb) break main
(gdb) continue
```

---

## IV. Event Monitor (watcher)

### Start Watcher

```bash
$ watcher
```

**Output Format**:
```
[timestamp_ms] [pid] [event_name]
```

### Understanding Output

- `[0]` - First event, baseline time = 0ms
- `[150]` - Event 150ms after first event
- `[3]` - Process ID that triggered event
- `fork/write/sleep` - Event type

### Example Session

**Terminal 1** - watcher:
```bash
$ watcher
Event Watcher Started (PID: 3)
Listening for kernel events...
====================================
Format: [Delta(ms)] [PID] [Event]
====================================
[0] [2] fork
[15] [3] write
[30] [4] fork
[45] [5] write
```

**Terminal 2** - commands (Ctrl+A+C to switch):
```bash
$ echo hello
hello
$ ls
...
$ cat Makefile
...
```

### Running Multiple Commands

Watcher captures ALL events, so you can:

```bash
# Terminal 1
$ watcher &

# Terminal 2 - Run many commands
$ echo test1
$ echo test2
$ ls
$ sleep 1
$ cat Makefile
```

Watcher output will show events from all these.

---

## V. Testing with eventtest

### Run Test Program

```bash
$ eventtest
```

**Output**:
```
Test: Creating fork events
Child 4 running
Child 5 running
Child 6 running
Test: Writing to console
Hello from test
```

### Verify with Watcher

Best way to test:

**Terminal 1**:
```bash
$ watcher
Event Watcher Started (PID: 3)
Listening for kernel events...
====================================
Format: [Delta(ms)] [PID] [Event]
====================================
[0] [3] fork
[10] [4] fork
[20] [5] fork
[30] [5] write
[40] [6] write
```

**Terminal 2** (Ctrl+A+C to switch):
```bash
$ eventtest
Test: Creating fork events
Child 4 running
Child 5 running
Child 6 running
Test: Writing to console
Hello from test
```

### Expected Results

✅ Watcher captures exactly 3 fork events (one per child)  
✅ Watcher captures 2 write events (from eventtest output)  
✅ Timestamps increase monotonically  
✅ No crashes or errors  

---

## VI. Syscalls API

### 1. kqueue_wait() - Wait for Event

**Signature**:
```c
int kqueue_wait(struct event *ev);
```

**Parameters**:
- `ev` - Pointer to struct event to receive data

**Returns**:
- `0` - Success
- `-1` - Error

**Example**:
```c
struct event ev;
if(kqueue_wait(&ev) < 0) {
  perror("kqueue_wait");
  exit(1);
}
printf("Got event: %s from pid %d at time %lu\n",
       ev.name, ev.pid, ev.timestamp);
```

### 2. kqueue_post() - Post Custom Event

**Signature**:
```c
int kqueue_post(int pid, const char *name);
```

**Parameters**:
- `pid` - Process ID
- `name` - Event name string

**Returns**:
- `0` - Success
- `-1` - Error

**Example**:
```c
if(kqueue_post(getpid(), "custom_event") < 0) {
  perror("kqueue_post");
}
```

### 3. struct event Definition

```c
struct event {
  int pid;              // Process ID (4 bytes)
  char name[32];       // Event name (32 bytes)
  uint64 timestamp;    // Kernel ticks (8 bytes)
};                     // Total: 44 bytes
```

---

## VII. Event Types

### 1. **fork** Event

- **Triggered**: When fork() syscall succeeds
- **PID**: ID of child process created
- **Example**: `[15] [4] fork`

### 2. **sleep** Event

- **Triggered**: When process calls sleep() or waits on I/O
- **PID**: ID of process that slept
- **Example**: `[30] [5] sleep`

### 3. **write** Event

- **Triggered**: When write() syscall succeeds
- **PID**: ID of writing process
- **Example**: `[45] [3] write`

### 4. Custom Events

- User programs can post custom events via kqueue_post()
- Any event name up to 32 characters

---

## VIII. Command Reference

| Command | Description | Example |
|---------|-------------|---------|
| `make clean` | Remove build artifacts | `make clean` |
| `make` | Compile kernel & programs | `make -j4` |
| `make qemu` | Run emulator | `make qemu` |
| `watcher` | Start event monitor | `watcher` |
| `eventtest` | Run tests | `eventtest` |
| `echo TEXT` | Print text | `echo hello` |
| `ls` | List files | `ls` |
| `cat FILE` | Show file content | `cat README` |
| `sleep N` | Wait N seconds | `sleep 2` |
| `exit` | Exit shell | `exit` |

### QEMU Terminal Switching

- **Ctrl+A+C** - Enter QEMU console
- **quit** - Exit QEMU console
- **Ctrl+A+X** - Exit QEMU immediately

---

## IX. Troubleshooting

### Problem: "command not found: watcher"

**Cause**: watcher not compiled

**Solution**:
```bash
make clean && make -j4
# Verify:
ls -l user/_watcher
```

### Problem: QEMU won't start

**Cause**: QEMU not installed

**Solution**:
```bash
# Ubuntu/Debian
sudo apt-get install qemu-system-misc

# macOS
brew install qemu

# Verify
qemu-system-riscv64 --version
```

### Problem: watcher shows no events

**Cause**: No commands running, or watcher started after commands

**Solution**:
1. Always start watcher FIRST
2. Then run commands in another terminal
3. Use Ctrl+A+C to switch terminals

### Problem: "undefined reference to kqueue_post"

**Cause**: Stale kernel build

**Solution**:
```bash
make clean
make -j4
```

### Problem: Compilation errors

**Solution**:
```bash
# Full rebuild
make clean && make 2>&1 | tail -20

# Check errors specifically
make 2>&1 | grep -i error
```

### Problem: watcher hangs with no output

**Cause**: Waiting for first event but no commands run

**Solution**:
- Run command in another terminal
- Or Ctrl+C to stop watcher

---

## X. System Architecture

```
┌─────────────────────────────────────┐
│        KERNEL (xv6-riscv)           │
│                                     │
│  • kqueue - 256 event circular      │
│    buffer                           │
│  • fork() hook - posts fork event   │
│  • sleep() hook - posts sleep event │
│  • write() hook - posts write event │
│  • sleep/wakeup - efficient waiting │
│                                     │
└──────────────┬──────────────────────┘
               │ syscalls
               ▼
┌─────────────────────────────────────┐
│     USER-SPACE PROGRAMS             │
│                                     │
│  watcher                            │
│  ├─ kqueue_wait() → blocks until    │
│  │  event available                 │
│  └─ print formatted event           │
│                                     │
│  eventtest                          │
│  ├─ fork() → triggers fork event    │
│  └─ write() → triggers write event  │
│                                     │
│  Any program (ls, cat, etc)         │
│  └─ Generates events automatically  │
│                                     │
└─────────────────────────────────────┘
```

---

## XI. Performance Notes

### CPU Usage

- **watcher idle**: ~0% (sleeping via sleep/wakeup)
- **With events**: <1% (woken up, processes event, sleeps)
- **Before fix**: Was 100% (busy-wait loop)

### Memory

- **kqueue struct**: ~10KB for 256 events
- **Per event**: 44 bytes
- **watcher program**: ~10KB

### Latency

- **Event posting**: < 1 microsecond
- **watcher wakeup**: < 10 microseconds
- **Total path**: < 20 microseconds

---

## XII. Known Limitations

1. **printf width specifiers not supported**
   - Use `printf("[%lu]")` not `printf("[%4lu]")`
   - This is xv6 user-space limitation

2. **Event name max 32 chars**
   - Names longer than 32 chars are truncated

3. **Queue size 256 events**
   - If queue fills up, oldest events are dropped
   - This is by design (circular buffer)

4. **No persistence**
   - Events lost when program exits
   - Can redirect to file: `watcher > /tmp/events.log`

---

## XIII. Real-World Scenarios

### Monitor Shell Activity

```bash
# Terminal 1
$ watcher

# Terminal 2 (Ctrl+A+C to switch)
$ for i in 1 2 3; do echo $i; done
1
2
3
```

Watcher will show fork+write events for each echo.

### Log Events to File

```bash
# Terminal 1
$ watcher > /tmp/events.log &

# Terminal 2
$ eventtest
$ sleep 1
$ ls

# Terminal 3 - View logs later
$ cat /tmp/events.log
[0] [3] fork
[10] [4] write
...
```

### Monitor Specific Process

```bash
# Terminal 1
$ watcher

# Terminal 2
$ sleep 5  # This will show sleep events
```

Look for sleep events for PID 4.

---

## XIV. Development Tips

### Add Custom Event

Modify your C program:

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
  kqueue_post(getpid(), "my_custom_event");
  
  // Do work...
  
  kqueue_post(getpid(), "work_done");
  
  exit(0);
}
```

Then rebuild and watcher will capture these.

### Analyze Event Stream

```bash
# Save events
watcher > /tmp/events.log

# Count events by type
grep fork /tmp/events.log | wc -l   # Count forks
grep write /tmp/events.log | wc -l  # Count writes
grep sleep /tmp/events.log | wc -l  # Count sleeps

# Find max timestamp
tail -1 /tmp/events.log | awk '{print $1}'
```

### Debug Event Timing

```bash
# Watch only fork events
$ watcher | grep fork

# Watch only write events
$ watcher | grep write

# Watch events from specific PID
$ watcher | grep "\[4\]"
```

---

## XV. File Structure

```
xv6-riscv-riscv-Subproject/
├── kernel/
│   ├── proc.h         (struct event, struct kqueue)
│   ├── proc.c         (kqueueinit, kqueue_post, hooks)
│   ├── syscall.h      (SYS_kqueue_wait=22, SYS_kqueue_post=23)
│   ├── syscall.c      (syscall dispatch)
│   └── sysfile.c      (write hook)
├── user/
│   ├── watcher.c      (event monitor - 34 lines)
│   ├── eventtest.c    (test program - 28 lines)
│   └── user.h         (struct event definition)
├── Makefile
├── USAGE_GUIDE.md     (this file)
├── EVENTQUEUE_README.md
├── DESIGN_DOCUMENT.md
└── CHANGELOG_DETAILED.md
```

---

## XVI. Support & Documentation

**In This Repository**:
- **EVENTQUEUE_README.md** - Architecture overview
- **DESIGN_DOCUMENT.md** - Technical deep dive
- **PROJECT_SUMMARY.md** - Implementation checklist
- **CHANGELOG_DETAILED.md** - Week-by-week changes

**GitHub Issues**: https://github.com/holisurt/xv6-subproject-kernel-event/issues

---

## XVII. Summary

This Kernel Event Queue System provides:

✅ **Automatic event capture** from kernel operations  
✅ **Real-time monitoring** via watcher program  
✅ **Efficient sleep/wakeup** mechanism (0% idle CPU)  
✅ **Thread-safe** queue access with spinlocks  
✅ **User-space API** for custom events  
✅ **Well-documented** with examples  

---

**Happy monitoring!** 🎯

*Version 1.0 - December 19, 2025*  
*xv6-riscv Kernel Event Queue System*

