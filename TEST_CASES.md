# Kernel Event Queue System - Test Cases & Guide

## Overview

This document provides 5+ comprehensive test cases for the xv6 Kernel Event Queue System. Each test demonstrates different aspects of the event capturing functionality.

---

## Test Case 1: Basic Fork Event Capture

**Objective**: Verify that fork events are captured with correct process IDs and timestamps.

**Setup**:
```bash
$ rm log1.txt 2>/dev/null; true
$ watcher -o log1.txt &
$ eventtest
$ cat log1.txt
```

**Expected Output**:
```
[526] [2] fork        # Shell forks
[141409] [5] fork     # eventtest child 1
[154677] [6] fork     # eventtest child 2
[165432] [7] fork     # eventtest child 3
```

**Success Criteria**:
- ✅ At least 4 fork events captured (shell + 3 children)
- ✅ Fork events appear in chronological order
- ✅ PID values are sequential (2, 5, 6, 7, etc.)
- ✅ Timestamps increase monotonically

**What This Tests**: Core fork event posting and timing functionality

---

## Test Case 2: Sleep Events with Wait Syscalls

**Objective**: Verify that sleep events are captured when processes wait for children.

**Setup**:
```bash
$ rm log2.txt 2>/dev/null; true
$ watcher -i -o log2.txt &
$ eventtest
$ cat log2.txt
```

**Expected Output**:
```
[526] [2] fork
[141401] [2] sleep      # Shell waiting after fork
[141409] [5] fork
[141468] [5] sleep      # Child 1 sleeping
[142210] [5] sleep      # Child 1 sleeping again
[154677] [6] fork
[154902] [6] sleep      # Child 2 sleeping
```

**Success Criteria**:
- ✅ Both fork and sleep events visible
- ✅ Sleep events occur after fork events
- ✅ Multiple sleep events for same PID (process sleeps multiple times)
- ✅ Sleep events properly logged with -i flag

**What This Tests**: Sleep event posting from internal sleep() function, fork/sleep correlation

---

## Test Case 3: Write Events Detection

**Objective**: Verify that write syscalls generate events.

**Setup**:
```bash
$ rm log3.txt 2>/dev/null; true
$ watcher -o log3.txt &
$ writetest
$ cat log3.txt
```

**Expected Output**:
```
[1000] [2] fork        # Shell forks writetest
[2500] [8] fork        # writetest forks child 1
[3200] [8] write       # writetest writes
[3500] [9] write       # child 1 writes
[4100] [8] fork        # writetest forks child 2
[4800] [10] write      # child 2 writes
[5200] [8] sleep       # parent waits
```

**Success Criteria**:
- ✅ Fork events visible
- ✅ Write events captured (from write syscall)
- ✅ Multiple write events from different PIDs
- ✅ Write events interspersed with fork/sleep

**What This Tests**: Write event posting, multi-process write tracking

---

## Test Case 4: Stress Test - Fork Bomb

**Objective**: Verify system handles high-frequency fork events without deadlock or data loss.

**Setup**:
```bash
$ rm log4.txt 2>/dev/null; true
$ watcher -i -o log4.txt &
$ forktest
$ wc -l log4.txt
$ grep fork log4.txt | wc -l
$ cat log4.txt | head -20
```

**Expected Output**:
```
$ wc -l log4.txt
100+  log4.txt          # Many events captured

$ grep fork log4.txt | wc -l
20+                     # Many fork events

$ cat log4.txt | head -20
[541] [2] fork
[1234] [3] fork
[2456] [4] fork
...
```

**Success Criteria**:
- ✅ System doesn't hang or crash
- ✅ 20+ fork events captured
- ✅ 50+ total events (forks + sleeps)
- ✅ All timestamps valid and increasing
- ✅ Output file readable and complete

**What This Tests**: Circular buffer overflow handling, system stability under load, event rate capacity

---

## Test Case 5: Contextual Event Display

**Objective**: Verify that contextual mode shows forks with related sleep/write events nearby.

**Setup**:
```bash
$ rm log5.txt 2>/dev/null; true
$ watcher -c 2 -o log5.txt &
$ eventtest
$ cat log5.txt
```

**Expected Output**:
```
[141401] [2] sleep
[141409] [5] fork
  ---
[141468] [5] sleep
[141614] [5] sleep
[154677] [6] fork
  ---
[154902] [6] sleep
[155305] [6] sleep
```

**Success Criteria**:
- ✅ Fork events clearly marked
- ✅ Related events appear before each fork
- ✅ Separator lines ("  ---") appear after each fork
- ✅ Events within 2-second window of fork shown
- ✅ Events outside window not shown

**What This Tests**: Event buffering, contextual filtering, time window logic

---

## Test Case 6: Multiple Rapid Forks

**Objective**: Verify accurate event capture during concurrent process creation.

**Setup**:
```bash
$ rm log6.txt 2>/dev/null; true
$ watcher -i -o log6.txt &
$ forktest
$ sleep 2
$ tail -50 log6.txt
```

**Expected Output**:
```
[193203] [6] fork
[193363] [7] fork
[193368] [8] fork
[193373] [9] fork
[193378] [10] fork
[193384] [11] fork
[193388] [12] fork
```

**Success Criteria**:
- ✅ Multiple fork events in rapid succession
- ✅ Timestamps show microsecond precision
- ✅ PIDs increment sequentially
- ✅ No event loss or corruption
- ✅ Proper event ordering maintained

**What This Tests**: High-frequency event posting, circular buffer management, timestamp accuracy

---

## Test Case 7: Event Queue Wraparound

**Objective**: Verify circular buffer correctly wraps around when full.

**Setup**:
```bash
$ rm log7.txt 2>/dev/null; true
$ watcher -i -o log7.txt &
$ forktest
$ forktest
$ wc -l log7.txt
$ tail -30 log7.txt | grep fork
```

**Expected Output**:
```
$ wc -l log7.txt
200+  log7.txt

$ tail -30 log7.txt | grep fork
[193660] [62] fork
[193864] [69] fork
[193922] [70] fork
```

**Success Criteria**:
- ✅ No deadlock or crash after 256 events
- ✅ Latest events preserved
- ✅ Circular buffer wraps correctly
- ✅ Event data not corrupted

**What This Tests**: NEVENT=256 buffer wraparound, circular buffer indexing

---

## Quick Reference: Running All Tests

**Test Suite Execution**:

```bash
# Clean up old logs
$ rm log*.txt 2>/dev/null; true

# Run all tests in sequence
$ watcher -i -o log1.txt &
$ eventtest
$ sleep 1

$ watcher -i -o log2.txt &
$ writetest
$ sleep 1

$ watcher -c 3 -o log3.txt &
$ forktest
$ sleep 1

# View results
$ wc -l log*.txt
$ grep fork log1.txt | wc -l
$ grep sleep log2.txt | wc -l
$ grep write log3.txt | wc -l
```

---

## Debugging Failed Tests

### Issue: No events captured

**Check**:
1. Is watcher running? `ps aux | grep watcher`
2. Is output file created? `ls -la log*.txt`
3. Use correct flags:
   - `-i` for important events (fork + sleep)
   - No flags for fork only
   - `-o FILE` for file output

**Fix**:
```bash
# Make sure to use -i for sleep events
$ watcher -i -o events.log &
```

### Issue: "exec eventtest failed"

**Check**:
1. Correct spelling: `eventtest` (not `eventest`)
2. Program is in filesystem: `ls eventtest`
3. Watcher is not interfering with exec

**Fix**:
```bash
# Type carefully
$ eventtest        # Correct
$ eventest         # WRONG - misspelled
```

### Issue: Only fork events, no sleep/write

**Check**:
1. Are you using `-i` flag? (required for sleep)
2. For write events, don't use `-i` flag

**Fix**:
```bash
$ watcher -i -o log.txt &   # For fork + sleep
$ watcher -o log.txt &      # For all events
```

### Issue: Output file very large (1000+ lines)

**Possible causes**:
1. Capturing too many small sleep events
2. Test ran too long
3. Using wrong watcher mode

**Fix**:
```bash
# Limit to important events
$ watcher -i -o log.txt &

# Or use contextual mode
$ watcher -c 5 -o log.txt &

# Or just fork events
$ watcher -o log.txt &
```

---

## Performance Notes

- **eventtest**: Fast, completes in <1 second, 3-4 fork events
- **writetest**: Medium, completes in <2 seconds, 3-4 fork events + write events
- **forktest**: Slow, completes in 5-10 seconds, 20+ fork events, high load

---

## Success Indicators

✅ **System is working correctly if**:
- All 3 test programs run without hanging
- Event logs are created and readable
- Events have increasing timestamps
- Fork events correspond to actual process creation
- No kernel panics or reboots

⚠️ **Issues to watch for**:
- System hangs during test (deadlock)
- Event log truncated or corrupted
- Timestamps not increasing
- Missing fork events
- Kernel panic messages

---

## Next Steps

After passing all tests:
1. Analyze event timing patterns
2. Correlate events with actual system behavior
3. Test with custom programs that use different syscalls
4. Monitor performance impact of event queue
5. Extend event types (e.g., exec, exit)
