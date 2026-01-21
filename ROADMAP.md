Phase 1: Foundation (Weeks 1-4)
Week 1-2: Understanding the Basics
What you'll learn:

What Linux is and how operating systems work
The command line (your new best friend)
Files, processes, and basic system concepts

Think of it like this:
Your Computer
    ├── Hardware (CPU, RAM, Disk)
    ├── Kernel (Linux - the core OS)
    └── User Space (where your programs live)
Key concepts:

Everything is a file in Linux (even devices!)
Processes are running programs
The shell is your control center

Week 3-4: C Programming Fundamentals
Why C? It's the language Linux is written in, and it gives you direct control over hardware.
Topics:

Variables, data types, and memory
Pointers (the most important concept!)
Functions and program structure
Compiling with gcc

Simple example:
```
// Your first program
#include <stdio.h>

int main() {
    printf("Hello, Systems World!\n");
    return 0;
}
```

---

## **Phase 2: Core Systems Programming (Weeks 5-12)**

### Week 5-6: Memory and Pointers Deep Dive
**Understanding memory layout:**
```
High Address
    ├── Command line args & environment
    ├── Stack (grows downward) ↓
    ├── ↓ (free space)
    ├── ↑ (free space)
    ├── Heap (grows upward) ↑
    ├── Uninitialized data (BSS)
    ├── Initialized data
    └── Text (program code)
Low Address
Key skills:

Stack vs Heap allocation
Manual memory management (malloc/free)
Understanding memory addresses
Debugging with valgrind

Week 7-8: File I/O and System Calls
The bridge between your program and the kernel:
c// System calls vs library functions
open()    // System call - direct to kernel
fopen()   // Library function - wrapper around open()
```

**What you'll learn:**
- File descriptors (think of them as handles)
- read(), write(), open(), close()
- Error handling with errno
- Buffered vs unbuffered I/O

### Week 9-10: Processes and Process Management
**Every program is a process:**
```
Parent Process (PID: 100)
    │
    ├── fork() → Child Process (PID: 101)
    │            └── exec() → New Program
    │
    └── wait() ← Waits for child to finish
```

**Topics:**
- Creating processes with fork()
- Replacing process images with exec()
- Process lifecycle and states
- Zombie and orphan processes

### Week 11-12: Inter-Process Communication (IPC)
**How processes talk to each other:**
```
Process A                Process B
    ├── Pipe ──────────────→ 
    ├── Shared Memory ←────→ 
    ├── Message Queue ←────→ 
    └── Signals ──────────→
```

**Key concepts:**
- Pipes (unnamed and named/FIFO)
- Shared memory (fastest IPC)
- Message queues
- Signals for simple notifications

---

## **Phase 3: Concurrency and Performance (Weeks 13-20)**

### Week 13-15: Multithreading with pthreads
**Threads: lightweight processes sharing memory:**
```
Process
    ├── Thread 1 (shares memory) ──┐
    ├── Thread 2 (shares memory) ──┤→ All access same data
    └── Thread 3 (shares memory) ──┘
Critical topics:

Creating and joining threads
Race conditions (when things go wrong!)
Mutexes and locks
Condition variables
Thread-local storage

Week 16-17: Synchronization Primitives
Protecting shared data:
c// Without mutex - DANGEROUS!
counter++;  // Thread 1 and Thread 2 might corrupt this!

// With mutex - SAFE!
pthread_mutex_lock(&mutex);
counter++;
pthread_mutex_unlock(&mutex);
```

**You'll master:**
- Mutexes vs Spinlocks
- Reader-writer locks
- Semaphores
- Atomic operations
- Deadlock prevention

### Week 18-20: I/O Multiplexing and Async I/O
**Handling thousands of connections efficiently:**
```
Single Thread Monitoring:
    ├── Socket 1 ──┐
    ├── Socket 2 ──┤
    ├── Socket 3 ──┼──→ epoll/select watches all
    ├── ... ───────┤
    └── Socket N ──┘
```

**Technologies:**
- select(), poll(), epoll()
- Non-blocking I/O
- Event-driven programming
- io_uring (modern Linux async I/O)

---

## **Phase 4: Low-Latency Optimization (Weeks 21-30)**

### Week 21-23: CPU Architecture and Cache Optimization
**Understanding modern CPUs:**
```
CPU Core
    ├── L1 Cache (fastest, ~1ns, 32-64KB)
    ├── L2 Cache (fast, ~3ns, 256KB-1MB)
    ├── L3 Cache (shared, ~12ns, 8-32MB)
    └── RAM (slow, ~60ns, GBs)
Critical concepts:

Cache lines (64 bytes typically)
False sharing (cache line ping-pong)
Memory alignment
Prefetching
Branch prediction

Example of false sharing:
c// BAD - threads fight over same cache line
struct {
    int counter1;  // Used by thread 1
    int counter2;  // Used by thread 2
} data;

// GOOD - separate cache lines
struct {
    int counter1;
    char padding[60];  // Force to different cache lines
    int counter2;
} data;
```

### Week 24-26: Lock-Free Programming
**Coordination without locks (advanced but essential for low-latency):**
```
Traditional:
    Lock → Access Data → Unlock (threads wait)

Lock-Free:
    Read → Compute → Compare-and-Swap (threads never block)
```

**Topics:**
- Atomic operations (CAS, fetch-and-add)
- Memory ordering (acquire/release semantics)
- Lock-free queues
- ABA problem
- Memory barriers

### Week 27-28: Real-Time Linux and Scheduling
**Controlling when your code runs:**
```
Scheduling Policies:
    SCHED_FIFO     (Real-time, first-in-first-out)
    SCHED_RR       (Real-time, round-robin)
    SCHED_DEADLINE (Guaranteed CPU time)
    SCHED_OTHER    (Normal processes)
```

**Key skills:**
- CPU affinity (pinning threads to cores)
- Priority inversion problems
- Real-time patches (PREEMPT_RT)
- Minimizing jitter
- CPU isolation

### Week 29-30: System-Level Optimization
**Squeezing out every microsecond:**

**Memory:**
- Huge pages (reduce TLB misses)
- NUMA awareness
- Memory locking (prevent swapping)

**Networking:**
- Kernel bypass (DPDK, raw sockets)
- Zero-copy techniques
- Interrupt handling optimization

**Example visualization:**
```
Normal Network Path:        Optimized Path:
    ┌──────────┐              ┌──────────┐
    │   NIC    │              │   NIC    │
    └────┬─────┘              └────┬─────┘
         │ Interrupt               │ Poll
    ┌────▼─────┐              ┌────▼─────┐
    │  Kernel  │              │User Space│
    └────┬─────┘              │ (bypass) │
         │ Copy               └──────────┘
    ┌────▼─────┐
    │User Space│
    └──────────┘
```

---

## **Phase 5: Profiling and Debugging (Weeks 31-34)**

### Week 31-32: Performance Analysis Tools
**Finding bottlenecks:**
```
Tool Stack:
    ├── perf (CPU profiling, hardware counters)
    ├── valgrind (memory analysis)
    ├── gdb (debugging)
    ├── strace (system call tracing)
    ├── ltrace (library call tracing)
    └── flamegraphs (visualization)
What you'll measure:

CPU cycles per operation
Cache miss rates
Branch mispredictions
Memory bandwidth
Latency percentiles (p50, p99, p99.9)

Week 33-34: Advanced Debugging
When things go wrong at 3 AM:

Core dumps analysis
Race condition detection (ThreadSanitizer)
Memory corruption tracking
Production debugging techniques
Writing bulletproof error handling


Phase 6: Practical Projects (Weeks 35-40)
Project 1: High-Performance Logger
Build a lock-free, multi-threaded logging system
Project 2: Zero-Copy Message Queue
Implement a shared memory circular buffer
Project 3: Low-Latency TCP Server
Use epoll and non-blocking I/O for thousands of connections
Project 4: Memory Pool Allocator
Replace malloc with a fast, custom allocator
