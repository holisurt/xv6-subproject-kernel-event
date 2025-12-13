# Hướng Dẫn Sử Dụng Kernel Event Queue System

## Tổng Quan

Kernel Event Queue System là một hệ thống theo dõi các sự kiện kernel trong xv6-riscv. 
Nó cho phép bạn theo dõi các hoạt động quan trọng như fork, sleep, và write operations 
một cách real-time từ user-space.

---

## I. Hướng Dẫn Build & Chạy

### 1. Build Kernel và User Programs

```bash
# Vào thư mục project
cd /home/duyen-hung/Desktop/OS/xv6-riscv-riscv-Subproject

# Clean build artifacts cũ (tùy chọn)
make clean

# Build kernel, user programs, và filesystem image
make -j4

# Hoặc build đơn giản (tuần tự)
make
```

**Kết quả mong đợi**:
- ✅ Không có lỗi compile
- ✅ Kernel được link thành công
- ✅ File system image (fs.img) được tạo ra
- ✅ User programs (watcher, eventtest) được compile

### 2. Chạy QEMU

```bash
# Khởi động QEMU với kernel xv6
make qemu
```

Nếu cần debug:
```bash
# Chạy với gdb
make qemu-gdb
```

**Output mong đợi**:
```
xv6 kernel is booting

hart 2 starting
hart 1 starting
hart 0 starting
init: starting sh
$ 
```

Bây giờ bạn đã sẵn sàng chạy các chương trình event queue!

---

## II. Sử Dụng Event Monitor (watcher)

### 1. Khởi Động Watcher

```bash
$ watcher
```

**Output**:
```
Event Watcher Started (PID: 3)
Listening for kernel events...
=====================================
Format: [Delta(ms)] [PID] [Event]
=====================================
```

Sau đó watcher sẽ chờ và in ra các event từ kernel.

### 2. Format Output

Mỗi event được in theo format:
```
[delta_time] [pid] [event_name]
```

Giải thích:
- **delta_time**: Thời gian tính từ event đầu tiên (milliseconds)
- **pid**: Process ID sinh ra event
- **event_name**: Tên sự kiện (fork, write, sleep)

### 3. Ví Dụ Chạy Watcher

**Terminal 1 - Khởi động watcher**:
```bash
$ watcher
Event Watcher Started (PID: 3)
Listening for kernel events...
=====================================
Format: [Delta(ms)] [PID] [Event]
=====================================
[   0] [  2] fork      
[  15] [  4] fork      
[  30] [  5] write     
[  45] [  3] write     
[  60] [  6] sleep     
```

**Terminal 2 - Chạy test program** (dùng Ctrl+A+C để switch terminal):
```bash
$ eventtest
Test: Creating fork events
Child 4 running
Child 5 running
Child 6 running
Test: Writing to console
Hello from test
```

Trong terminal 1, bạn sẽ thấy watcher in ra tất cả các event!

---

## III. Chạy Các Command Khác Cùng Với Watcher

### 1. Test với `echo`

```bash
# Terminal 1 - Start watcher
$ watcher

# Terminal 2 - Execute commands
$ echo hello
hello
```

**Watcher output**:
```
[   0] [  3] fork      
[  10] [  4] write     
```

### 2. Test với `ls`

```bash
# Terminal 1 - Start watcher
$ watcher

# Terminal 2
$ ls
cat    dorphan  echo  eventtest  forktest  grep   init   kill  ln
ls     mkdir    rm    sh         stressfs  usertests watcher wc zombie
```

**Watcher output**:
```
[   0] [  3] fork      
[  15] [  4] write     
[  25] [  5] write     
[  35] [  6] write
```

### 3. Test với `sleep`

```bash
# Terminal 1 - Start watcher
$ watcher

# Terminal 2
$ sleep 2
```

**Watcher output**:
```
[   0] [  3] fork      
[  10] [  4] sleep     
[  2100] [  4] write    
```

Chú ý: sleep event sẽ được capture khi process gọi `sleep`.

### 4. Test với Multiple Programs

```bash
# Terminal 1
$ watcher

# Terminal 2
$ echo "Test 1"
Test 1

$ ls
...

$ cat Makefile
...
```

**Watcher output sẽ show tất cả events từ các command:**
```
[   0] [  3] fork      
[  10] [  4] write     
[  20] [  3] fork      
[  30] [  5] write     
[  40] [  3] fork      
[  50] [  6] write     
```

---

## IV. Chạy Unit Test (eventtest)

### 1. Khởi Động Test Program

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

### 2. Chạy Test với Watcher

**Cách tốt nhất để verify hệ thống:**

```bash
# Terminal 1 - Start watcher trước
$ watcher
Event Watcher Started (PID: 3)
Listening for kernel events...
=====================================
Format: [Delta(ms)] [PID] [Event]
=====================================

# Terminal 2 - Start eventtest (Ctrl+A+C để switch)
$ eventtest
Test: Creating fork events
Child 4 running
Child 5 running
Child 6 running
Test: Writing to console
Hello from test
```

**Watcher sẽ capture**:
```
[   0] [  2] fork      
[  10] [  3] fork      
[  20] [  4] fork      
[  30] [  5] write     
[  40] [  3] fork      
[  50] [  4] write     
[  60] [  5] write     
```

### 3. Kiểm Tra Kết Quả

- ✅ Mỗi fork() sinh ra 1 "fork" event
- ✅ Mỗi write() sinh ra 1 "write" event
- ✅ Delta time tăng dần theo thời gian thực
- ✅ Không có crash hoặc lỗi

---

## V. Các Lệnh Syscall Available

### 1. kqueue_wait() - Đợi Event

Chức năng: Chờ event từ kernel queue

```c
int kqueue_wait(struct event *ev);
```

**Tham số**:
- `ev`: Pointer tới struct event (để nhận dữ liệu event)

**Return**:
- 0: Thành công
- -1: Lỗi

**Ví dụ**:
```c
struct event ev;
kqueue_wait(&ev);
printf("Event: %s from pid %d at time %lu\n", ev.name, ev.pid, ev.timestamp);
```

### 2. kqueue_post() - Post Event

Chức năng: Post event từ user-space

```c
int kqueue_post(int pid, const char *name);
```

**Tham số**:
- `pid`: Process ID
- `name`: Tên event (chuỗi)

**Return**:
- 0: Thành công
- -1: Lỗi

**Ví dụ**:
```c
kqueue_post(getpid(), "custom_event");
```

---

## VI. Event Types

Hệ thống hiện hỗ trợ các event type sau:

### 1. **fork** - Process Creation
- **Khi**: Process con được tạo thành công
- **PID**: ID của process con
- **Ví dụ output**: `[15] [4] fork`

### 2. **sleep** - Process Sleep
- **Khi**: Process gọi sleep() (chờ I/O, timer, etc)
- **PID**: ID của process đang sleep
- **Ví dụ output**: `[30] [5] sleep`

### 3. **write** - Write Operation
- **Khi**: Process gọi write() thành công
- **PID**: ID của process writing
- **Ví dụ output**: `[45] [3] write`

### 4. **Custom Events**
- Người dùng có thể post custom event từ user-space
- Dùng `kqueue_post(pid, "custom_name")`

---

## VII. Bảng Tham Khảo Lệnh

| Lệnh | Mô Tả | Ví Dụ |
|------|--------|--------|
| `make clean` | Xóa build artifacts | `make clean` |
| `make` | Build kernel & programs | `make -j4` |
| `make qemu` | Chạy QEMU | `make qemu` |
| `watcher` | Start event monitor | `watcher` |
| `eventtest` | Run test program | `eventtest` |
| `echo <text>` | Print text | `echo hello` |
| `ls` | List files | `ls` |
| `sleep <n>` | Sleep n seconds | `sleep 2` |
| `exit` | Exit shell | `exit` |
| `Ctrl+A+C` | Switch terminal (QEMU) | - |

---

## VIII. Troubleshooting

### 1. Lỗi: "Command not found: watcher"

**Nguyên nhân**: watcher chưa được build

**Giải pháp**:
```bash
make clean
make
make qemu
# Sau đó chạy watcher
```

### 2. QEMU không khởi động

**Nguyên nhân**: QEMU chưa được install

**Giải pháp**:
```bash
# Linux (Ubuntu/Debian)
sudo apt-get install qemu-system-misc

# macOS
brew install qemu

# Hoặc download từ https://www.qemu.org/download/
```

### 3. watcher không nhận event

**Nguyên nhân**: Chạy watcher sau khi chạy command khác

**Giải pháp**: 
- Luôn khởi động watcher TRƯỚC
- Sau đó mới chạy các command khác

### 4. Lỗi compile: "undefined reference to `kqueue_post`"

**Nguyên nhân**: Kernel code cũ

**Giải pháp**:
```bash
make clean
make -j4
```

### 5. Timeout hoặc hang

**Nguyên nhân**: watcher chờ event nhưng không có command running

**Giải pháp**:
- Chạy command trong terminal khác
- Hoặc press Ctrl+C để stop watcher

---

## IX. Kiến Trúc Hệ Thống

```
┌──────────────────────────────────┐
│      KERNEL (xv6-riscv)          │
│                                  │
│  Global kqueue (256 slots)       │
│  ├─ fork event hook              │
│  ├─ sleep event hook             │
│  └─ write event hook             │
│                                  │
└──────────────┬───────────────────┘
               │ (syscalls)
               ▼
┌──────────────────────────────────┐
│     USER-SPACE PROGRAMS          │
│                                  │
│  ┌──────────────────────────┐   │
│  │  watcher.c               │   │
│  │  - Call kqueue_wait()    │   │
│  │  - Display events        │   │
│  │  - Real-time monitor     │   │
│  └──────────────────────────┘   │
│                                  │
│  ┌──────────────────────────┐   │
│  │  eventtest.c             │   │
│  │  - Generate fork events  │   │
│  │  - Generate write events │   │
│  │  - Test system           │   │
│  └──────────────────────────┘   │
│                                  │
│  ┌──────────────────────────┐   │
│  │  Other programs          │   │
│  │  (echo, ls, sleep, etc)  │   │
│  │  Automatically generate  │   │
│  │  events                  │   │
│  └──────────────────────────┘   │
│                                  │
└──────────────────────────────────┘
```

---

## X. Các Scenario Thực Tế

### Scenario 1: Monitor Shell Activity

```bash
# Terminal 1
$ watcher
Event Watcher Started (PID: 3)
Listening for kernel events...

# Terminal 2
$ ls
...
$ echo test
test
$ pwd
/
$ mkdir /tmp/test
```

**Watcher output**:
```
[   0] [  3] fork
[  10] [  4] write
[  20] [  3] fork
[  30] [  5] write
[  40] [  3] fork
[  50] [  6] write
[  60] [  3] fork
[  70] [  7] write
```

### Scenario 2: Analyze Process Behavior

```bash
# Terminal 1
$ watcher > /tmp/events.log &

# Terminal 2
$ eventtest
$ cat /tmp/events.log
```

Output file sẽ chứa tất cả events, có thể phân tích sau.

### Scenario 3: Performance Monitoring

```bash
# Monitor fork rate
$ watcher &
$ forktest
```

Bạn sẽ thấy tất cả fork events được sinh ra, giúp phân tích hiệu suất.

---

## XI. Cấu Trúc Event Data

Mỗi event chứa:

```c
struct event {
  int pid;                   // Process ID (4 bytes)
  char name[32];            // Event name (32 bytes)
  uint64 timestamp;         // Kernel ticks (8 bytes)
};
// Total: 44 bytes per event
```

### Ví dụ:
```
Event 1: { pid: 4, name: "fork", timestamp: 1000 }
Event 2: { pid: 5, name: "write", timestamp: 1100 }
Event 3: { pid: 6, name: "sleep", timestamp: 1200 }
```

---

## XII. Giới Hạn & Limitations

| Thông Số | Giá Trị | Ghi Chú |
|----------|--------|--------|
| Max events | 256 | Circular buffer |
| Event name | 32 chars | Fixed size |
| Queue access | Thread-safe | Spinlock |
| Performance | O(1) | Constant time |
| CPU usage | Zero | Sleep/wakeup |

---

## XIII. Tài Liệu Tham Khảo

- **EVENTQUEUE_README.md**: Tổng quan dự án
- **DESIGN_DOCUMENT.md**: Chi tiết technical
- **PROJECT_SUMMARY.md**: Tóm tắt implementation

---

## XIV. Liên Hệ & Support

Nếu có vấn đề:

1. Check các lệnh trong phần **Troubleshooting**
2. Verify build thành công: `make clean && make`
3. Restart QEMU: `make qemu`
4. Read documentation files trong project

---

**Chúc bạn sử dụng hệ thống Event Queue thành công!** ✨

---

*Last Updated: December 13, 2025*  
*xv6-riscv Kernel Event Queue System v1.0*
