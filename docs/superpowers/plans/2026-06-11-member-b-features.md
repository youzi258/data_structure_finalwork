# Member B Features Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add the member B algorithm and extension modules for matching, hash-based lookup, location statistics, and menu integration.

**Architecture:** Keep member B code in focused modules that read `ItemList` through public headers. Matching owns its own result linked list; hash lookup owns a temporary chained hash index; statistics owns a temporary location-count list. The main menu calls these modules without transferring ownership of member A records.

**Tech Stack:** C11, MinGW GCC at `D:/mingw64/bin/gcc.exe`, MinGW Make at `D:/mingw64/bin/mingw32-make.exe`, standard C library only.

---

### Task 1: Matching Results

**Files:**
- Create: `include/match.h`
- Create: `src/match.c`
- Create: `tests/test_match.c`
- Modify: `Makefile`

- [ ] Write failing tests for 100-point scoring, thresholded result generation, and descending result order.
- [ ] Run `D:\mingw64\bin\mingw32-make.exe test-match` and verify it fails because the API is missing.
- [ ] Implement scoring, reason text, result linked-list generation, sorting, and clearing.
- [ ] Run `D:\mingw64\bin\mingw32-make.exe test-match` and verify it passes.

### Task 2: Hash Lookup

**Files:**
- Create: `include/hash_index.h`
- Create: `src/hash_index.c`
- Create: `tests/test_hash_index.c`
- Modify: `Makefile`

- [ ] Write failing tests for case-insensitive category lookup, keyword token lookup, and collision handling.
- [ ] Run `D:\mingw64\bin\mingw32-make.exe test-hash` and verify it fails because the API is missing.
- [ ] Implement chained hash buckets that store non-owning `const Item *` references.
- [ ] Run `D:\mingw64\bin\mingw32-make.exe test-hash` and verify it passes.

### Task 3: Location Statistics

**Files:**
- Create: `include/statistics.h`
- Create: `src/statistics.c`
- Create: `tests/test_statistics.c`
- Modify: `Makefile`

- [ ] Write failing tests for counting repeated lost-item locations and sorting by descending count.
- [ ] Run `D:\mingw64\bin\mingw32-make.exe test-statistics` and verify it fails because the API is missing.
- [ ] Implement the location-count linked list and clear function.
- [ ] Run `D:\mingw64\bin\mingw32-make.exe test-statistics` and verify it passes.

### Task 4: Menu Integration And Data

**Files:**
- Modify: `src/main.c`
- Modify: `data/items.csv`
- Modify: `Makefile`

- [ ] Replace the member B placeholder with a submenu for generating matches, hash search, and location statistics.
- [ ] Add more representative data so the demo covers at least 15 lost and 15 found records.
- [ ] Run `D:\mingw64\bin\mingw32-make.exe test` and `D:\mingw64\bin\mingw32-make.exe app`.
- [ ] Run a scripted smoke test that loads CSV data, enters the member B menu, exercises each option, and exits.

### Task 5: Publish

**Files:**
- Modify only files required by verification findings.

- [ ] Run clean verification from a fresh build.
- [ ] Commit member B work.
- [ ] Push `codex/member-b-features` over SSH.
- [ ] Use `C:\Program Files\GitHub CLI\gh.exe` to create a PR targeting `main`.
