# Enhanced Watcher Tool Guide

The `watcher` tool is a comprehensive kernel event monitoring application for xv6 that captures system events in real-time. It supports multiple filtering modes to understand process behavior.

## Current Status

✅ **Fully Functional** - All 3 modes working without deadlock
✅ **Sleep Events** - Captured from both sys_pause and internal sleep()
✅ **Write Events** - Captured from write syscalls
✅ **Fork Events** - Captured with accurate PIDs and timestamps
✅ **File Output** - Silent mode for background operation

## Modes

### Mode 0: Fork Events Only (Default)
Shows only fork events. This is the most minimal and fastest mode.

```bash
$ watcher -o events.log &
$ eventtest
$ cat events.log
[40] [2] fork
[92108] [3] fork
[133954] [5] fork
[154677] [6] fork
```

**Use case**: Quick overview of process creation, minimal overhead
**Output**: Fork events only, clean and concise

---

### Mode 1: Important Events (`-i` flag)
Shows fork events AND sleep events from wait() syscalls. Filters out write events.

```bash
$ watcher -i -o events.log &
$ eventtest
$ cat events.log
[40] [2] fork
[141401] [2] sleep
[141409] [5] fork
[141468] [5] sleep
[154677] [6] fork
[154902] [6] sleep
```

**Use case**: Tracking both process creation and blocking/waiting behavior
**Output**: Fork + sleep events interleaved

---

### Mode 2: Contextual Events (`-c` flag)
Shows fork events with **related sleep/write events** that occurred nearby within a time window.

**Syntax**: `watcher -c [SECONDS] -o events.log &`

```bash
# Default context window: 10 seconds
$ watcher -c -o events.log &

# Custom context window: 5 seconds
$ watcher -c 5 -o events.log &

# Custom context window: 1 second
$ watcher -c 1 -o events.log &
```

**Output format** with separators for clarity:
```
[event_time] [pid] sleep
[event_time] [pid] write
[fork_time] [pid] fork
  ---
[next_fork_time] [pid] fork
  ---
```

**Use case**: Understanding what operations surround each fork, detailed analysis
**Output**: Grouped events with visual separators

---

## Event Types

The system captures three types of events:

| Event | Source | Description |
|-------|--------|-------------|
| **fork** | fork() syscall | Process creation - parent PID when child is created |
| **sleep** | wait(), pause(), and internal sleep() | Process blocking/sleeping on I/O or locks |
| **write** | write() syscall | File/console I/O operations |

**Visibility in each mode**:
- Mode 0 (default): fork only
- Mode 1 (-i): fork + sleep
- Mode 2 (-c): fork + sleep + write (with filtering)

---

## Complete Examples

### Example 1: Fork events only
```bash
$ watcher -o events.log &
$ eventtest
$ cat events.log
```
**Output**: Fork events from eventtest and children
**Use**: Process creation timeline

### Example 2: Fork + Sleep events
```bash
$ watcher -i -o events.log &
$ eventtest
$ cat events.log
```
**Output**: Shows fork events and sleep events from wait() calls
**Use**: Understanding blocking behavior

### Example 3: Write events
```bash
$ watcher -o events.log &
$ writetest
$ cat events.log
```
**Output**: Fork, write, and sleep events all mixed
**Use**: I/O pattern analysis

### Example 4: Stress test with contextual view
```bash
$ watcher -c 5 -o events.log &
$ forktest
$ cat events.log
```
**Output**: Fork events grouped with related events from 5 seconds before
**Use**: Understanding system behavior during heavy load

---

## Test Programs

Three test programs are available to demonstrate the event queue:

| Program | Purpose | Duration | Output |
|---------|---------|----------|--------|
| **eventtest** | Fork + wait demo | <1s | 3 forks + sleep events |
| **writetest** | Write event demo | <2s | 3 forks + write events |
| **forktest** | Stress test | 5-10s | 20+ forks + many sleeps |

---

## Flags Summary

| Flag | Purpose | Example |
|------|---------|---------|
| `-i` | Show important events (fork + sleep) | `watcher -i -o log.txt &` |
| `-c [N]` | Show contextual events (forks with nearby sleep/write within N seconds) | `watcher -c 5 -o log.txt &` |
| `-o FILE` | Write output to file instead of console | `watcher -o events.log &` |

**Flags can be combined**:
- `watcher -i -o log.txt &` - Important events to file
- `watcher -c 3 -o log.txt &` - Contextual events (3 sec window) to file

---

## Tips

1. **Silent background mode**: Always use `-o FILE` when running `watcher` in background to avoid console mixing
2. **Context windows**: Smaller windows (1-2 sec) for tight coupling, larger (5-10 sec) for loose correlations
3. **Real-time viewing**: Use `watcher` without `-o` flag and let output stream to console (use `&` to background it)

---

## Performance Notes

- Mode 0 (fork-only): Very fast, minimal overhead
- Mode 1 (important): Moderate overhead from sleep event tracking
- Mode 2 (contextual): Buffers all events, may use more memory for long-running processes

For production monitoring, use Mode 0 or 1. Use Mode 2 for detailed analysis on demand.
