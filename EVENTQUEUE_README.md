# Kernel Event Queue System for xv6-riscv

## Tổng Quan

Đây là một hệ thống theo dõi event (event queue) kernel được xây dựng cho OS xv6-riscv. 
Hệ thống cho phép kernel tự động ghi nhận và lưu trữ các sự kiện quan trọng 
(fork, write, sleep) và cung cấp một công cụ user-space để theo dõi chúng real-time.

## Kiến Trúc Hệ Thống

### 1. Kernel Components (kernel/)

#### `proc.h`
- **struct event**: Định nghĩa cấu trúc event gồm:
  - `pid`: Process ID sinh ra event
  - `name`: Tên event (fork, write, sleep)
  - `timestamp`: Thời điểm event xảy ra (kernel ticks)

- **struct kqueue**: Hàng đợi sự kiện kernel gồm:
  - `lock`: Spinlock bảo vệ truy cập queue
  - `events[NEVENT]`: Mảng lưu trữ tối đa 256 event
  - `head`, `tail`, `count`: Con trỏ và bộ đếm
  - `wakeup_lock`, `wakeup_flag`: Hỗ trợ sleep/wakeup thay vì busy-wait

#### `proc.c`
- **kqueueinit()**: Khởi tạo hàng đợi event
- **kqueue_post()**: Thêm event vào hàng đợi
  - Xử lý tràn queue (FIFO - discard oldest)
  - Ghi lại timestamp tự động
  - Gửi wakeup signal cho process đang chờ
  
- **kqueue_wait_internal()**: Chờ event từ hàng đợi
  - Sử dụng sleep/wakeup cơ chế
  - Tránh busy-wait để tiết kiệm CPU

- **sys_kqueue_wait()**: Syscall wrapper cho user-space
- **sys_kqueue_post()**: Syscall wrapper cho user-space post event

#### `sysfile.c` (Modifications)
- **sys_write()**: Được hook để post "write" event khi write thành công
- Giúp theo dõi hoạt động I/O của process

#### `syscall.c`/`syscall.h`
- Thêm 2 syscall number mới:
  - SYS_kqueue_wait (22)
  - SYS_kqueue_post (23)
- Mapping vào handler functions

### 2. User-Space Components (user/)

#### `watcher.c` - Event Monitoring Tool
```bash
$ watcher
Event Watcher Started (PID: 3)
Listening for kernel events...
=====================================
Format: [Delta(ms)] [PID] [Event]
=====================================
[   0] [  2] fork      
[  15] [  3] fork      
[  30] [  4] write     
```

Tính năng:
- Chạy liên tục lắng nghe event từ kernel queue
- In ra các event theo format: [delta_time] [pid] [event_name]
- Tính toán delta time từ event đầu tiên
- Không sử dụng busy-wait, được OS schedule tối ưu

#### `eventtest.c` - Test Program
```bash
$ eventtest
Test: Creating fork events
Child 4 running
Child 5 running
Child 6 running
Test: Writing to console
Hello from test
```

Tạo các event test:
- Fork 3 child process (sinh ra 3 fork event)
- Write data (sinh ra write event)
- Test tính đúng đắn của system

#### `usys.pl` & `usys.S`
- Sinh ra assembly stubs cho 2 syscall mới
- Cho phép user-space gọi kqueue_wait và kqueue_post

## Cơ Chế Hoạt Động

### Event Posting Flow
```
fork() / sleep() / sys_write()
    ↓
kqueue_post("event_name", pid)
    ↓
1. Acquire queue lock
2. Add event to circular queue
3. Set wakeup flag
4. Wakeup any sleeping processes
5. Release lock
```

### Event Waiting Flow
```
kqueue_wait() system call
    ↓
sys_kqueue_wait()
    ↓
kqueue_wait_internal()
    ↓
1. Acquire lock
2. While queue empty:
   - Sleep on wakeup_flag
   - Waken by kqueue_post()
3. Copy event to user buffer
4. Update head pointer
5. Return to user
```

## Tối Ưu Hóa (Week 7)

### 1. Sleep/Wakeup Instead of Busy-Wait
- **Trước**: kqueue_wait_internal() sử dụng busy-wait loop
  ```c
  for(volatile int i = 0; i < 1000000; i++);  // CPU waste
  ```
- **Sau**: Sử dụng kernel's sleep/wakeup mechanism
  ```c
  sleep(&kq.wakeup_flag, &kq.wakeup_lock);
  ```
- **Lợi ích**: Giảm CPU usage, process được schedule lại khi có event

### 2. Improved Output Formatting
- Tính toán delta time từ event đầu tiên
- Format: `[delta_ms] [pid] [event_name]`
- Dễ đọc và phân tích theo thời gian

### 3. Circular Queue Management
- Kích thước cố định (256 events)
- Tự động discard oldest event khi queue full
- Đảm bảo no memory leak

### 4. Timestamp Collection
- Mỗi event lưu kernel timestamp (r_time())
- Cho phép tính toán thời gian tương đối

## Syscall Interface

### kqueue_wait
```c
int kqueue_wait(struct event *ev);
```
- **Tham số**: Pointer đến struct event
- **Return**: 0 on success, -1 on error
- **Behavior**: Block cho đến khi có event, copy vào buffer user

### kqueue_post
```c
int kqueue_post(int pid, const char *name);
```
- **Tham số**: 
  - pid: Process ID
  - name: Event name string
- **Return**: 0 on success, -1 on error
- **Behavior**: Post event vào kernel queue

## Kernel Hooks

### fork() - kernel/proc.c
```c
// Sets up child kernel stack to return as if from fork() system call.
int kfork(void)
{
  ...
  // Post fork event to kernel queue
  kqueue_post("fork", pid);
  return pid;
}
```

### sleep() - kernel/proc.c
```c
void sleep(void *chan, struct spinlock *lk)
{
  ...
  p->state = SLEEPING;
  
  // Post sleep event to kernel queue
  kqueue_post("sleep", p->pid);
  ...
}
```

### sys_write() - kernel/sysfile.c
```c
uint64 sys_write(void)
{
  ...
  int result = filewrite(f, p, n);
  
  // Post write event to kernel queue
  if(result > 0) {
    kqueue_post("write", myproc()->pid);
  }
  return result;
}
```

## Build & Run

### Build
```bash
cd /home/duyen-hung/Desktop/OS/xv6-riscv-riscv-Subproject
make clean
make
```

### Run QEMU
```bash
make qemu
```

### Inside QEMU
```bash
# In one terminal, start watcher
$ watcher

# In another (use Ctrl+A+C to switch terminals in QEMU)
$ eventtest
```

## Testing

### Unit Test - eventtest.c
```bash
$ eventtest
Test: Creating fork events
Child 4 running
Child 5 running
Child 6 running
Test: Writing to console
Hello from test
```
Expected events captured by watcher:
- 3 fork events (one per child)
- 1 write event (from printf or explicit write)

### Integration Test - Multiple Programs
```bash
# Terminal 1
$ watcher

# Terminal 2 (switch with Ctrl+A+C)
$ echo hello
$ ls
$ sleep 2
```

Expected output in watcher:
```
[   0] [  3] fork      
[  10] [  4] write     
[  20] [  5] fork      
[  30] [  6] write     
```

## File Modifications Summary

| File | Change | Reason |
|------|--------|--------|
| kernel/proc.h | Added struct event, struct kqueue | Define event queue data structure |
| kernel/proc.c | Added kqueueinit(), kqueue_post(), kqueue_wait_internal(), sys_kqueue_wait(), sys_kqueue_post() | Core event queue implementation |
| kernel/proc.c | Modified fork() | Post fork event |
| kernel/proc.c | Modified sleep() | Post sleep event |
| kernel/sysfile.c | Modified sys_write() | Post write event |
| kernel/syscall.h | Added SYS_kqueue_wait, SYS_kqueue_post | Define syscall numbers |
| kernel/syscall.c | Added syscall handlers | Register new syscalls |
| kernel/defs.h | Added struct event forward declaration | Type safety |
| user/user.h | Added struct event, function declarations | User-space API |
| user/usys.pl | Added entry() calls for new syscalls | Generate syscall stubs |
| user/watcher.c | New file | Event monitoring tool |
| user/eventtest.c | New file | Test program |
| Makefile | Added _watcher, _eventtest to UPROGS | Build new programs |

## Performance Considerations

1. **Memory**: Queue size cố định (256 events × 40 bytes ≈ 10KB)
2. **CPU**: Sử dụng sleep/wakeup, không busy-wait
3. **Latency**: Post event là O(1), Wait event là O(1) khi có event
4. **Throughput**: Có thể xử lý 256 events trước overwrite

## Future Enhancements

1. Dynamic queue sizing
2. Per-process event filtering
3. Event timestamping với microsecond precision
4. Persistent logging to disk
5. Event statistics and analytics
6. Real-time process monitoring dashboard
7. Custom event types từ user programs
8. Event replay functionality

## Kết Luận

Hệ thống event queue kernel đã được thực hiện thành công với:
- ✅ Kernel event posting mechanism
- ✅ User-space event monitoring via syscalls
- ✅ Optimized sleep/wakeup instead of busy-wait
- ✅ Real-time monitoring tool (watcher)
- ✅ Test programs (eventtest)
- ✅ Proper synchronization dengan spinlock
- ✅ Circular buffer management

Hệ thống có thể được sử dụng để debug, monitor, và analyze kernel behavior 
trong xv6 một cách hiệu quả và real-time.
