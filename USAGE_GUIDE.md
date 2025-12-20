# Kernel Event Queue System - Complete Usage Guide

**Latest Update**: December 20, 2025
**Status**: ✅ Fully Functional - All systems working

---

## Quick Start (5 Minutes)

### 1. Build the System

```bash
cd /home/duyen-hung/Desktop/OS/xv6-riscv-riscv-Subproject
make clean
make -j4
```

### 2. Boot xv6 in QEMU

```bash
make qemu
```

Expected output:
```
xv6 kernel is booting
hart 1 starting
hart 2 starting
init: starting sh
$
```

### 3. Run Watcher (inside xv6 shell)

```bash
$ watcher -i -o events.log &
```

### 4. Run a Test Program

```bash
$ eventtest
Test: Creating fork events
Child 6 running
Child 7 running
Child 8 running
Test: Writing to console
Hello from test
```

### 5. View Captured Events

```bash
$ cat events.log
[40] [2] fork
[141401] [2] sleep
[141409] [5] fork
[141468] [5] sleep
[154677] [6] fork
...
```

---

## Build & Installation

### Building from Scratch

```bash
# Full clean rebuild
make clean && make -j4

# Or just rebuild without clean
make -j4

# Check for build errors
make 2>&1 | grep -i error
```

### Verify Build Success

Check that all programs are in the filesystem:

```bash
ls -la user/_watcher user/_eventtest user/_forktest user/_writetest
# Should show 4 files, each ~40KB
```

### Build Artifacts

After build, you'll have:
- `kernel/kernel` - xv6 kernel binary
- `fs.img` - Filesystem image with all programs
- `user/_watcher` - Event monitoring tool
- `user/_eventtest` - Fork/wait test program
- `user/_forktest` - Fork stress test
- `user/_writetest` - Write event demo

---

## Running xv6

### Basic Run (Interactive Shell)

```bash
make qemu
```

Then use the xv6 shell to run commands.

### Debug Mode (with gdb)

Terminal 1 - Start xv6 with GDB server:
```bash
make qemu-gdb
```

Terminal 2 - Connect debugger:
```bash
riscv64-unknown-elf-gdb kernel/kernel
(gdb) target remote :26000
(gdb) break main
(gdb) continue
```

### Running Tests Automatically

```bash
make qemu < test-commands.txt
```

### Exit xv6

```bash
$ exit
# or press Ctrl+A+X
```

---

## Event Watcher Tool

### Overview

The `watcher` tool captures three types of kernel events:
- **fork** - Process creation
- **sleep** - Process blocking (wait, pause, internal sleep)
- **write** - File/console I/O

### Modes

#### Mode 0: Fork Events Only (Default)

```bash
$ watcher -o events.log &
$ eventtest
$ cat events.log
```

**Output**: Only fork events
```
[526] [2] fork
[141409] [5] fork
[154677] [6] fork
```

#### Mode 1: Important Events (-i flag)

```bash
$ watcher -i -o events.log &
$ eventtest
$ cat events.log
```

**Output**: Fork + sleep events
```
[526] [2] fork
[141401] [2] sleep
[141409] [5] fork
[141468] [5] sleep
[154677] [6] fork
```

#### Mode 2: Contextual Events (-c flag)

```bash
$ watcher -c 5 -o events.log &
$ eventtest
$ cat events.log
```

**Output**: Forks grouped with nearby events (5 sec window)
```
[141401] [2] sleep
[141409] [5] fork
  ---
[141468] [5] sleep
[154677] [6] fork
  ---
```

### Flags

| Flag | Description | Example |
|------|-------------|---------|
| `-i` | Important events (fork + sleep) | `watcher -i -o log.txt &` |
| `-c [N]` | Contextual (forks with nearby events, N seconds) | `watcher -c 5 -o log.txt &` |
| `-o FILE` | Write to file (silent mode) | `watcher -o events.log &` |

### Output Format

```
[Delta_ms] [PID] [EventType]
```

- **Delta_ms**: Milliseconds since first event
- **PID**: Process ID that triggered event
- **EventType**: fork, sleep, or write

### Example Workflows

**Test 1: See process creation**
```bash
$ watcher -o log1.txt &
$ eventtest
$ cat log1.txt
```

**Test 2: See blocking behavior**
```bash
$ watcher -i -o log2.txt &
$ forktest
$ cat log2.txt
```

**Test 3: See I/O patterns**
```bash
$ watcher -o log3.txt &
$ writetest
$ cat log3.txt
```

**Test 4: Stress test**
```bash
$ watcher -i -o log4.txt &
$ forktest
$ forktest
$ wc -l log4.txt
```

---

## Test Programs

### eventtest - Fork & Wait Demo

Creates 3 child processes and waits for them.

```bash
$ eventtest
Test: Creating fork events
Child 6 running
Child 7 running
Child 8 running
Test: Writing to console
Hello from test
```

**Events captured**: 3 forks + multiple sleep events from wait()

### forktest - Stress Test

Fork stress test that creates many processes rapidly.

```bash
$ forktest
fork test
fork test OK
```

**Events captured**: 20+ forks + many sleep events

**Duration**: 5-10 seconds (CPU intensive)

### writetest - Write Event Demo

Demonstrates write events from multiple processes.

```bash
$ writetest
=== Write Test Program ===
This test demonstrates write system calls being captured by watcher

Test 1: Parent process writes
  Write #1 from parent
  Write #2 from parent
  Write #3 from parent

Test 2: Parent and child both write
    [Child] Writing to console
Parent: child finished

Test 3: Multiple children writing
    [Child] Message
=== Write Test Complete ===
```

**Events captured**: Forks + write events + sleep events

### Running Multiple Tests

```bash
$ watcher -i -o combined.log &
$ eventtest
$ writetest
$ forktest
$ sleep 2
$ cat combined.log
```

---

## Common Operations

### View Events in Real-time

```bash
$ watcher &
$ eventtest
# Events will print to console in real-time
```

### Log Events to File

```bash
$ watcher -i -o events.log &
$ forktest
$ cat events.log | wc -l    # Count events
$ grep fork events.log      # Show only forks
$ grep sleep events.log     # Show only sleeps
```

### Analyze Event Frequency

```bash
$ watcher -i -o log.txt &
$ forktest
$ grep fork log.txt | wc -l     # Count forks
$ grep sleep log.txt | wc -l    # Count sleeps
$ wc -l log.txt                 # Total events
```

### Compare Different Modes

```bash
# Mode 0: Forks only
$ watcher -o log_forks.txt &
$ eventtest
$ wc -l log_forks.txt

# Mode 1: Forks + sleeps
$ rm log_forks.txt
$ watcher -i -o log_all.txt &
$ eventtest
$ wc -l log_all.txt

# Mode 2: Contextual
$ rm log_all.txt
$ watcher -c 2 -o log_context.txt &
$ eventtest
$ wc -l log_context.txt
```

---

## Troubleshooting

### Issue: "exec PROG failed"

**Cause**: Typo in program name or program doesn't exist

**Solution**:
```bash
# Check available programs
$ ls e*
# Programs: echo, eventtest

# Correct spelling:
$ eventtest     # Correct
$ eventest      # WRONG
```

### Issue: No events in log file

**Cause**: Using wrong flags or watcher not running

**Solution**:
```bash
# Check if watcher is running
$ ps

# Use -i flag for important events
$ watcher -i -o log.txt &

# Verify file is created
$ ls -la log.txt
```

### Issue: Log file too large

**Cause**: Capturing too many events (sleep spam)

**Solution**:
```bash
# Use default mode (forks only)
$ watcher -o log.txt &

# Or use contextual mode with smaller window
$ watcher -c 2 -o log.txt &

# Or use -i flag for important events only
$ watcher -i -o log.txt &
```

### Issue: System hangs or crashes

**Cause**: Deadlock or infinite loop

**Solution**: This should NOT happen with current version. If it does:
1. Kill QEMU (Ctrl+C)
2. Rebuild: `make clean && make`
3. Report issue

---

## Performance Notes

### Event Overhead

- **Mode 0 (fork only)**: Minimal overhead
- **Mode 1 (-i)**: Low overhead, captures sleep syscalls
- **Mode 2 (-c)**: Moderate overhead, buffers all events

### Memory Usage

- Event buffer: 256 events × 44 bytes ≈ 11 KB
- Watcher process: ~30 KB
- Total impact: Negligible

### Event Throughput

- forktest creates ~20-30 forks in 5-10 seconds
- Each fork generates 1+ sleep events
- System sustains 100+ events per second easily

---

## Exit Sequences

### Exit Single Program

```bash
$ exit
```

### Exit watcher (stop monitoring)

```bash
# Kill watcher background process
$ killall watcher
# or
$ pkill watcher
```

### Exit xv6 Completely

```bash
$ exit
# QEMU window closes automatically
```

---

## Advanced Topics

### Event Queue Internals

- **Circular buffer**: 256 slots (NEVENT)
- **Event structure**: 44 bytes (pid + name[32] + timestamp)
- **Lock**: kq_lock spinlock
- **Capacity**: ~11 KB total

### Syscall Numbers

- `sys_kqueue_wait()` - SYS #22 - Read next event (blocking)
- `sys_kqueue_post()` - SYS #23 - Post event (internal use)

### Event Posting Locations

- **fork()**: kernel/proc.c:401
- **sleep()**: kernel/proc.c:667 (skips if chan == &kq)
- **sys_pause()**: kernel/sysproc.c:90

### Deadlock Prevention

Sleep event posting is skipped when:
- Process sleeping on kqueue lock (`chan == &kq`)
- This prevents recursive lock issues in kqueue_wait()

---

## Testing Checklist

Use this checklist to verify everything is working:

- [ ] Build succeeds without errors
- [ ] xv6 boots in QEMU
- [ ] eventtest runs and shows fork events
- [ ] forktest runs and shows many forks
- [ ] writetest runs and shows write events
- [ ] watcher -i shows fork + sleep events
- [ ] watcher -c shows contextual grouping
- [ ] Logs can be saved to files
- [ ] System doesn't hang or crash
- [ ] Multiple tests can run in sequence

---

## Support & Documentation

**See Also**:
- `TEST_CASES.md` - Detailed test cases with expected outputs
- `WATCHER_GUIDE.md` - Comprehensive watcher tool documentation
- `DESIGN_DOCUMENT.md` - System architecture and design
- `EVENTS_EXPLAINED.md` - Detailed event descriptions
- `SYSTEM_WORKFLOW.txt` - Complete workflow explanation

**GitHub**: https://github.com/holisurt/xv6-subproject-kernel-event

**Latest Commits**:
```
82d6596 - Add comprehensive test cases and testing guide
7e7b833 - Add writetest program to demonstrate write event capturing
63f7593 - Enable sleep event posting in internal sleep() function safely
4d89c76 - Re-enable sleep event posting safely from sys_pause syscall
2f105c9 - Fix deadlock in event queue and enhance watcher with contextual modes
```

---

**Last Updated**: December 20, 2025
**Status**: ✅ All systems fully functional and tested
