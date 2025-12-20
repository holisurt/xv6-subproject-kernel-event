# Enhanced Watcher Tool Guide

The `watcher` tool now supports three event filtering modes to help you understand kernel events in xv6.

## Modes

### Mode 0: Fork Events Only (Default)
Shows only fork events. This is the most minimal output.

```bash
$ watcher -o events.log &
$ eventtest
$ cat events.log
[40] [2] fork
[92108] [3] fork
[133954] [5] fork
...
```

**Use case**: Quick overview of process creation.

---

### Mode 1: Important Events (`-i` flag)
Shows fork and sleep events only. Filters out write events.

```bash
$ watcher -i -o events.log &
$ eventtest
$ cat events.log
[40] [2] fork
[92108] [3] fork
[133954] [5] fork
...
```

**Use case**: Tracking both process creation and blocking behavior.

---

### Mode 2: Contextual Events (`-c` flag)
Shows fork events with **related sleep/write events** that occurred nearby.

**Syntax**: `watcher -c [SECONDS] -o events.log &`

```bash
# Default context window: 10 seconds
$ watcher -c -o events.log &

# Custom context window: 5 seconds
$ watcher -c 5 -o events.log &

# Custom context window: 1 second
$ watcher -c 1 -o events.log &
```

**Output format**:
```
[event_time] [pid] sleep
[event_time] [pid] write
[fork_time] [pid] fork
  ---
[next_fork_time] [pid] fork
  ---
```

**Use case**: Understanding what system calls happen around process creation.

---

## Complete Examples

### Example 1: Basic fork tracking
```bash
$ watcher -o events.log &
$ eventtest
$ cat events.log
```

Shows when each child process (6, 7, 8) was created.

### Example 2: Detailed process behavior
```bash
$ watcher -i -o events.log &
$ eventtest
$ forktest
$ cat events.log
```

Shows forks and sleeps, but no I/O spam.

### Example 3: System call context
```bash
$ watcher -c 2 -o events.log &
$ eventtest
$ cat events.log
```

Shows what sleep/write operations happened within 2 seconds before each fork.

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
