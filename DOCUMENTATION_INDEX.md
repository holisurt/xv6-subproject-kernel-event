# Kernel Event Queue System - Complete Documentation Index

## Project Overview

This is an **xv6-riscv implementation** with a **Kernel Event Queue System** that monitors and captures system events (fork, sleep, write) for educational and debugging purposes.

**Repository:** https://github.com/holisurt/xv6-subproject-kernel-event

---

## 📚 Documentation Files

### 1. **USAGE_GUIDE.md** - How to Use (START HERE!)
Complete guide for using the system:
- Quick start (5 minutes)
- Building the project
- Running QEMU
- Using the watcher tool
- Program naming convention (important!)
- Watcher options: `-i` (filtering), `-o` (file output)
- Testing procedures
- Syscalls API reference
- Troubleshooting

**👉 Start here if you want to RUN the system**

---

### 2. **EVENTS_EXPLAINED.md** - Understanding Events (NEW!)
In-depth explanation of what each event means:
- **fork event** - Process creation
  - What is fork?
  - Code examples
  - Real-world examples
  - Why it's important

- **sleep event** - Process waiting/blocking
  - What is sleep?
  - I/O waiting examples
  - Performance implications
  - Use cases

- **write event** - Data output
  - What is write?
  - Console/file output examples
  - Pipe communication
  - Use cases

- Visual timeline examples
- Implementation details
- FAQ

**👉 Start here if you want to UNDERSTAND the events**

---

### 3. **DESIGN_DOCUMENT.md** - Architecture & Implementation
Technical deep-dive into system design:
- Architecture overview
- Circular buffer implementation
- Syscalls: kqueue_wait() and kqueue_post()
- Thread safety with spinlocks
- Integration with kernel (fork, sleep, write hooks)
- Event structure and timing
- Performance considerations

**👉 Start here if you want to UNDERSTAND the implementation**

---

### 4. **CHANGELOG_DETAILED.md** - Development History
Week-by-week implementation timeline (Weeks 2-7):
- Week 2: Kernel event queue design
- Week 3: Event posting in fork/sleep/write
- Week 4: User-space syscalls
- Week 5: Watcher application
- Week 6: Event filtering
- Week 7: Integration and testing

**👉 Read if you want to TRACK the development**

---

### 5. **EVENTQUEUE_README.md** - Event Queue Subsystem
Focused documentation on the event queue:
- Circular buffer design
- Spinlock synchronization
- Kernel-space implementation
- User-space interface
- Memory layout

**👉 Read if you want SUBSYSTEM details**

---

### 6. **PROJECT_SUMMARY.md** - High-Level Overview
Executive summary and project status:
- Project goals
- What was implemented
- Key features
- Testing results
- Known limitations
- Future improvements

**👉 Read for a QUICK OVERVIEW**

---

## 🚀 Quick Start Paths

### Path 1: I want to RUN and TEST the system
1. Read: **USAGE_GUIDE.md** (sections I-IV)
2. Run: `make qemu`
3. Try: `watcher -i -o events.log &` then `eventtest`
4. Learn: Read **EVENTS_EXPLAINED.md** to understand what you're seeing

### Path 2: I want to UNDERSTAND the events
1. Read: **EVENTS_EXPLAINED.md** (complete)
2. Run: `make qemu` and try capturing events
3. Analyze: Look at your events.log and match against the explanations

### Path 3: I want to UNDERSTAND the implementation
1. Read: **DESIGN_DOCUMENT.md** (complete)
2. Study: Source code in `kernel/proc.c`, `kernel/syscall.h`
3. Read: **EVENTQUEUE_README.md** for subsystem details
4. Review: **CHANGELOG_DETAILED.md** for development timeline

### Path 4: I want COMPLETE understanding
1. Start with **PROJECT_SUMMARY.md** (overview)
2. Read **USAGE_GUIDE.md** (sections I-II for setup)
3. Read **DESIGN_DOCUMENT.md** (complete architecture)
4. Read **EVENTS_EXPLAINED.md** (what events mean)
5. Run `make qemu` and test
6. Review **CHANGELOG_DETAILED.md** (development history)

---

## 🔧 Files Changed Recently

### Documentation Updates (Latest)
- **EVENTS_EXPLAINED.md** - NEW! Comprehensive event explanation (477 lines)
- **USAGE_GUIDE.md** - Updated with program naming and watcher options

### Code Updates
- **user/watcher.c** - Added `-o filename` option for file logging
- **user/watcher.c** - Increased delay to 100ms for better output separation
- **Makefile** - Removed broken _printftest target

### Bug Fixes
- Fixed printf width specifier issue (removed %4lu, %3d, etc.)
- Fixed watcher compilation error (sleep() not available in user-space)
- Implemented polling-based delay using uptime() syscall

---

## 📋 Key Commands

### Build & Run
```bash
cd /home/duyen-hung/Desktop/OS/xv6-riscv-riscv-Subproject
make clean && make -j4
make qemu
```

### Testing (in QEMU)
```bash
# Important! Use program names WITHOUT underscore
watcher -i -o events.log &    # Monitor important events to file
eventtest                      # Generate fork events
forktest                        # Fork stress test
cat events.log                  # View captured events
```

### Git Operations
```bash
git log --oneline              # View commit history
git show HEAD                  # View latest commit
git push origin main           # Push to GitHub
```

---

## 🎯 Program Naming Convention (IMPORTANT!)

**Build files** have underscores:
- `user/_watcher`
- `user/_eventtest`
- `user/_forktest`

**QEMU shell commands** DO NOT:
- `watcher` (not `_watcher`)
- `eventtest` (not `_eventtest`)
- `forktest` (not `_forktest`)

**Why?** The underscore prevents host OS from running xv6 binaries.

---

## 📊 Event Types Reference

| Event | Triggered By | What It Shows | When to Use |
|-------|--------------|---------------|------------|
| **fork** | fork() syscall | Process creation | Debug spawning issues |
| **sleep** | sleep()/I/O wait | Process blocking | Find bottlenecks |
| **write** | write()/printf() | Data output | Track activity |

---

## 🔍 Watcher Tool Options

```bash
# Show all events (verbose)
watcher

# Show only fork and sleep (filtered)
watcher -i

# Log to file (prevents console mixing)
watcher -o events.log

# Best practice: combine options
watcher -i -o events.log &
```

---

## 📈 System Architecture

```
xv6 Kernel
├── Process Management (proc.c)
│   ├── fork() - posts "fork" event
│   ├── sleep() - posts "sleep" event
│   └── Context switching
├── File System (sysfile.c)
│   ├── write() - posts "write" event
│   └── File operations
└── Event Queue System
    ├── Circular buffer (256 events max)
    ├── Spinlock protection
    └── kqueue_wait()/kqueue_post() syscalls

User Space
├── watcher - Monitor events from queue
├── eventtest - Generate test events
├── forktest - Fork stress test
└── Other programs
```

---

## ✅ What's Implemented

- ✅ Circular event queue in kernel
- ✅ Event posting in fork, sleep, write
- ✅ User-space syscalls (kqueue_wait, kqueue_post)
- ✅ Watcher monitoring tool
- ✅ Event filtering (-i flag)
- ✅ File output (-o flag)
- ✅ Test programs (eventtest, forktest)
- ✅ Comprehensive documentation

---

## 🐛 Known Issues & Solutions

| Issue | Solution |
|-------|----------|
| Output mixing in background | Use `watcher -i -o file.log &` |
| Can't find program | Use correct name without underscore |
| printf format strings broken | Use %lu, %d instead of %4lu, %3d |
| Mostly sleep events in log | Clear log before running: `rm events.log` |

---

## 📞 Testing Workflow

**Complete test session:**

```bash
$ make qemu
[QEMU starts]

$ rm events.log
$ watcher -i -o events.log &
$ sleep 1                      # Let watcher initialize
$ eventtest                    # Creates 3 fork events
$ echo "Hello"                 # Creates 1 fork + write
$ cat events.log               # View results

Expected output:
[0] [2] fork
[50] [7] fork  
[100] [8] fork
[150] [9] fork
[200] [4] fork
[...more events...]
```

---

## 🔗 Related Files

**Source Code:**
- `kernel/proc.c` - Process management and event posting
- `kernel/proc.h` - Process structures and event queue
- `kernel/syscall.h` - Syscall definitions
- `user/user.h` - User-space syscall declarations
- `user/watcher.c` - Event monitoring program

**Documentation:**
- `README` - Original xv6 README
- `USAGE_GUIDE.md` - How to use the system
- `EVENTS_EXPLAINED.md` - What events mean (this file explains them)
- `DESIGN_DOCUMENT.md` - Technical design
- `CHANGELOG_DETAILED.md` - Development history

---

## 📝 Notes

- All events include timestamp (kernel ticks) and process ID
- Circular buffer holds max 256 events
- Events are synchronized with spinlock
- File output prevents console mixing with background watcher
- `-i` flag filters for important events only (fork, sleep)

---

## 🎓 Educational Value

This implementation teaches:
- How kernel systems work
- Process creation and management
- I/O and synchronization
- Circular buffer data structures
- System call implementation
- Event-driven programming

---

**Last Updated:** December 19, 2025  
**Repository:** https://github.com/holisurt/xv6-subproject-kernel-event  
**Status:** Complete and fully functional
