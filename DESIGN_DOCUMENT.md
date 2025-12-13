# Kernel Event Queue System - Design Document

## Executive Summary

Dự án này triển khai một hệ thống event queue trong kernel xv6-riscv 
để ghi nhận và theo dõi các sự kiện kernel quan trọng như fork, sleep, và write.

**Thời gian thực hiện**: 7 tuần (Tuần 2-7)
**Ngôn ngữ**: C (kernel) + C (user-space)
**Kiến trúc**: RISC-V 64-bit

## Phase Breakdown

### Week 2: Infrastructure Setup
**Mục tiêu**: Xây dựng cơ sở hạ tầng kernel queue

**Thực hiện**:
1. Định nghĩa `struct event`:
   - pid: Process ID
   - name[32]: Event name (fork, write, sleep)
   - timestamp: Kernel tick

2. Định nghĩa `struct kqueue`:
   - Circular buffer: events[256]
   - head, tail, count
   - spinlock lock
   - (Sau này) wakeup mechanism

3. Hàm khởi tạo:
   - kqueueinit(): Initialize kernel queue
   - Phải được gọi từ procinit()

4. Hàm posting:
   - kqueue_post(name, pid): Add event
   - Xử lý queue full (FIFO)

5. Kernel hook trong fork():
   - Sau khi set RUNNABLE
   - kqueue_post("fork", child_pid)

**Kết quả**: Kernel queue hoạt động, lưu được fork events

**Thách thức**:
- Chính xác circular buffer logic (head/tail calculation)
- Thread-safe với spinlock

### Week 3: Syscall Interface
**Mục tiêu**: User-space có thể lấy event từ kernel

**Thực hiện**:
1. Thêm syscall number:
   - SYS_kqueue_wait = 22
   - SYS_kqueue_post = 23

2. Kernel-side syscall handlers:
   - sys_kqueue_wait(): Fetch event từ queue
   - Lấy pointer từ user-space argument
   - Copy event data từ kernel sang user buffer

3. User-side wrappers:
   - usys.pl: Sinh ra assembly stubs
   - user.h: Declare syscall prototypes
   - struct event definition cho user programs

4. Syscall mapping:
   - syscall.c: Register handlers
   - syscall.h: Define SYS_* constants

**Kết quả**: 
- User programs có thể call kqueue_wait()
- Nhận event từ kernel queue

**Thách thức**:
- Argument passing từ user to kernel
- Memory safety - copyout/copyin
- Syscall number allocation

### Week 4: User-side Posting
**Mục tiêu**: User programs cũng có thể post events

**Thực hiện**:
1. sys_kqueue_post() handler
   - Lấy pid, name từ arguments
   - Call kernel kqueue_post()
   - Basic validation

2. User-space wrapper
   - Declare trong user.h
   - Syscall stub được sinh ra tự động

3. Basic testing
   - Viết test code gọi both wait/post
   - Verify events được lưu/lấy đúng

**Kết quả**: 
- Bi-directional event system
- Kernel và user-space đều có thể post

**Thách thức**:
- Argument parsing (int + string)
- Safe string handling với safestrcpy

### Week 5: Real Event Hooks
**Mục tiêu**: Tự động capture kernel events

**Thực hiện**:
1. Hook vào sleep():
   - Sau khi set p->state = SLEEPING
   - kqueue_post("sleep", p->pid)

2. Hook vào sys_write():
   - Sau successful write
   - kqueue_post("write", myproc()->pid)

3. Fork hook (từ Week 2) được kiểm tra lại

**Kết quả**: 
- Kernel tự động ghi các system calls quan trọng
- Không cần explicit posting từ user

**Lợi ích**:
- Transparent monitoring
- No user-space overhead

### Week 6: Monitoring Tool
**Mục tiêu**: Viết tool để theo dõi events real-time

**Thực hiện**:
```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
  struct event ev;
  uint64 start = 0;
  
  while(1) {
    kqueue_wait(&ev);  // Block until event
    
    if(start == 0)
      start = ev.timestamp;
    
    uint64 delta = (ev.timestamp - start) / 1000;  // Convert to ms
    printf("[%lu] [%d] %s\n", delta, ev.pid, ev.name);
  }
}
```

**Tính năng**:
- Lắng nghe kernel queue continuously
- In event với delta timestamp
- Format rõ ràng

**Build**:
- Thêm _watcher vào UPROGS trong Makefile
- Biên dịch như user program thông thường

### Week 7: Optimization & Polish
**Mục tiêu**: Tối ưu performance, thêm features

**Thực hiện**:

#### 1. Sleep/Wakeup instead of Busy-Wait
**Problem**: Original kqueue_wait_internal() dùng busy-wait
```c
while(kq.count == 0) {
  release(&kq.lock);
  for(volatile int i = 0; i < 1000000; i++);  // CPU waste!
  acquire(&kq.lock);
}
```

**Solution**: Sử dụng kernel's sleep/wakeup
```c
while(kq.count == 0) {
  release(&kq.lock);
  acquire(&kq.wakeup_lock);
  sleep(&kq.wakeup_flag, &kq.wakeup_lock);
  release(&kq.wakeup_lock);
  acquire(&kq.lock);
}
```

**Changes**:
- Add `wakeup_lock`, `wakeup_flag` to struct kqueue
- In kqueue_post(): Set flag + wakeup after adding event
- In kqueue_wait_internal(): Sleep on flag

**Benefits**:
- 100% reduction in busy-wait CPU usage
- Process context switches properly
- Better overall system performance

#### 2. Timestamp & Formatting
- Print delta time từ first event
- Format: `[delta_ms] [pid] [event_name]`
- Easier to read and correlate events

#### 3. Testing
- Viết eventtest.c: create fork/write events
- Verify watcher nhận đúng events
- Integration testing với multiple programs

#### 4. Documentation
- EVENTQUEUE_README.md: Project overview
- Detailed design document (file này)
- Usage examples

## Technical Deep Dive

### Circular Buffer Implementation
```c
struct kqueue {
  spinlock lock;
  event events[256];
  int head, tail, count;
};

// Adding event
void kqueue_post(const char *name, int pid) {
  acquire(&lock);
  
  if(count >= 256) {
    head = (head + 1) % 256;  // Discard oldest
    count--;
  }
  
  events[tail] = {pid, name, time()};
  tail = (tail + 1) % 256;
  count++;
  
  release(&lock);
}

// Reading event
void kqueue_wait_internal(event *ev) {
  acquire(&lock);
  
  while(count == 0) {
    // Sleep/wakeup code
  }
  
  *ev = events[head];
  head = (head + 1) % 256;
  count--;
  
  release(&lock);
}
```

### Synchronization
- **Queue access**: Spinlock
- **Sleep/Wakeup**: Condition variable via wakeup_flag
- **Race conditions**: Protected by both locks as needed

### Memory Safety
- safestrcpy() for string copying
- copyout() to write event to user space
- Fixed-size buffers (MAXEVENTNAME=32)

## Integration Points

### Fork System (kernel/proc.c: kfork)
- Called when fork() system call completes
- Posts fork event with child PID
- Zero overhead if queue full (just overwrites)

### Sleep System (kernel/proc.c: sleep)
- Called before process sleeps
- Posts sleep event with own PID
- Allows monitoring of I/O waits

### Write System (kernel/sysfile.c: sys_write)
- Called after successful write
- Posts write event with own PID
- Tracks all write operations

## Usage Scenarios

### Scenario 1: Monitor Process Creation
```bash
$ watcher &
$ echo hello
$ ls
$ exit
```

Expected output:
- fork event for shell
- write events for echo/ls output
- Any other system calls

### Scenario 2: Performance Analysis
```bash
$ watcher &
$ ./slow_program
```

Analyze event pattern to see:
- When process sleeps (I/O waits)
- When it writes (output)
- Timing between events

### Scenario 3: Debug System Behavior
```bash
$ watcher > /tmp/events.log
$ ./myapp
$ kill %1  # Stop watcher
$ cat /tmp/events.log
```

Post-mortem analysis of what happened

## Limitations & Future Work

### Current Limitations
1. Fixed queue size (256 events)
2. Simple circular buffer (no filtering)
3. Basic event types (fork, write, sleep)
4. Timestamps in kernel ticks (not wall-time)
5. No disk logging

### Future Enhancements
1. **Dynamic queue**: Grow as needed
2. **Filtering**: Per-process, per-event-type
3. **More events**: exec, exit, signal, etc.
4. **Better timestamps**: Wall-clock time
5. **Logging**: Write to disk/file
6. **Statistics**: Count, frequency, timing
7. **GUI**: Real-time visualization
8. **Replay**: Recreate event sequence

## Build Instructions

### Prerequisites
- RISC-V 64 GCC toolchain
- QEMU 7.2+
- Make

### Build
```bash
cd xv6-riscv-riscv-Subproject
make clean
make
```

### Run
```bash
make qemu

# Inside QEMU:
watcher &
eventtest
```

## Code Statistics

| Component | LOC | Files |
|-----------|-----|-------|
| Kernel modifications | ~150 | proc.c, proc.h, sysfile.c, syscall.c |
| User programs | ~100 | watcher.c, eventtest.c |
| Makefile changes | ~5 | Makefile |
| **Total** | **~255** | **~8 files** |

## References

- xv6 kernel source
- RISC-V ABI
- Linux eventfd/perf event inspiration

---

**Author**: Implementation Team  
**Date**: December 2025  
**Status**: Complete & Tested
