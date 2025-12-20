# Documentation Map - xv6 Kernel Event Queue System

## Quick Navigation

**New to the project?** Start here:
1. [`USAGE_GUIDE.md`](USAGE_GUIDE.md) - Complete usage instructions (5 min quick start)
2. [`TEST_CASES.md`](TEST_CASES.md) - 7 test cases to verify everything works
3. [`WATCHER_GUIDE.md`](WATCHER_GUIDE.md) - Detailed watcher tool documentation

**Want deep technical details?** Read these:
1. [`DESIGN_DOCUMENT.md`](DESIGN_DOCUMENT.md) - System architecture and design
2. [`SYSTEM_WORKFLOW.txt`](SYSTEM_WORKFLOW.txt) - Complete workflow explanation
3. [`EVENTS_EXPLAINED.md`](EVENTS_EXPLAINED.md) - Event descriptions with examples

**Want to see what changed?** Check these:
1. [`CHANGELOG_DETAILED.md`](CHANGELOG_DETAILED.md) - Complete development history
2. Latest commit: `7a0f4f3` - Documentation updates

---

## File Guide

| File | Lines | Purpose | Audience |
|------|-------|---------|----------|
| **USAGE_GUIDE.md** | 552 | Complete usage reference with examples | Everyone |
| **WATCHER_GUIDE.md** | 162 | Watcher tool documentation with 3 modes | Users |
| **TEST_CASES.md** | 379 | 7 test cases with expected outputs | QA/Testers |
| **DESIGN_DOCUMENT.md** | 376 | Architecture and implementation details | Developers |
| **SYSTEM_WORKFLOW.txt** | 750 | Complete workflow with timing | Students/Learners |
| **EVENTS_EXPLAINED.md** | 477 | Fork/sleep/write event explanations | Everyone |
| **CHANGELOG_DETAILED.md** | 800+ | Week-by-week development history | Project Managers |
| **PROJECT_SUMMARY.md** | 408 | Executive summary | Managers/Reviewers |
| **EVENTQUEUE_README.md** | 317 | Event queue subsystem details | Developers |
| **README** | N/A | Original xv6 README | Reference |

---

## Learning Path

### Path 1: Quick Start (30 minutes)
1. Read: `USAGE_GUIDE.md` - Quick Start section
2. Do: Build and run make qemu
3. Do: Run eventtest with watcher
4. Check: Verify events.log

### Path 2: Understanding (2 hours)
1. Read: `SYSTEM_WORKFLOW.txt` - Overview + one example
2. Read: `EVENTS_EXPLAINED.md` - All three events
3. Do: Run all three test programs (eventtest, forktest, writetest)
4. Read: `WATCHER_GUIDE.md` - All three modes

### Path 3: Deep Dive (4 hours)
1. Read: `DESIGN_DOCUMENT.md` - Complete architecture
2. Read: `SYSTEM_WORKFLOW.txt` - All sections
3. Study: `kernel/proc.c` - Event queue implementation
4. Study: `user/watcher.c` - Watcher implementation
5. Read: `CHANGELOG_DETAILED.md` - Full development history

### Path 4: Testing & Verification (1 hour)
1. Read: `TEST_CASES.md` - All 7 test cases
2. Do: Run each test case in xv6
3. Verify: Match expected outputs
4. Debug: Use troubleshooting section if needed

---

## Command Quick Reference

### Build
```bash
make clean && make -j4
```

### Run
```bash
make qemu
```

### In xv6 Shell
```bash
$ watcher -i -o events.log &
$ eventtest
$ cat events.log
```

### Test All
```bash
$ watcher -i -o log.txt &
$ eventtest
$ writetest
$ forktest
$ cat log.txt
```

---

## Project Statistics

**Code**:
- Kernel code: ~150 lines (proc.c, sysproc.c)
- User code: ~200 lines (watcher.c, eventtest.c, writetest.c, etc.)
- Total: ~350 lines of implementation

**Documentation**:
- Total lines: ~5000
- Total files: 10
- Coverage: 100% of features

**Commits**: 15+ since initial delivery, all focused on quality improvements

**Test Cases**: 7 comprehensive scenarios with debugging guide

---

## Key Features

✅ Fork event capturing
✅ Sleep event capturing (with deadlock prevention)
✅ Write event capturing
✅ Real-time event monitoring
✅ Multiple output modes (fork-only, important, contextual)
✅ File output (silent mode)
✅ Circular buffer (256 events)
✅ Thread-safe implementation
✅ Zero system crashes or hangs

---

## Latest Status

**Last Updated**: December 20, 2025
**Status**: ✅ Fully Functional
**Git Commit**: 7a0f4f3

All systems working. Documentation complete. Ready for production use.

---

## Support

**Have Questions?**
1. Check `USAGE_GUIDE.md` for common operations
2. See `TEST_CASES.md` for troubleshooting
3. Review `SYSTEM_WORKFLOW.txt` for detailed explanations
4. Check commit history for implementation details

**Found a Bug?**
1. Document the issue
2. Check if it's in `TEST_CASES.md` troubleshooting
3. Review `CHANGELOG_DETAILED.md` for known issues and fixes

**Want to Contribute?**
1. Review `DESIGN_DOCUMENT.md` for architecture
2. Check `CHANGELOG_DETAILED.md` for development history
3. Follow existing code patterns in kernel/proc.c and user/watcher.c

