# Linux POSIX IPC Concurrent File Server

A multi-process client-server file engine written in C using System V / POSIX Inter-Process Communication primitives on Linux.

## Repository Structure

```text
posix-ipc-file-server/
├── Makefile
├── README.md
├── cliente.c      # Client process submitting file requests and reading responses via per-PID FIFO
├── server.c       # Concurrent server managing shared memory, semaphores, and child worker processes
└── server.h       # Shared data structures, IPC keys, and protocol definitions
```

## Key Engineering Highlights

- **Shared Memory & Synchronization:** Uses System V Shared Memory segments for low-overhead request transfer paired with semaphores to enforce producer-consumer mutual exclusion.
- **Isolated Response Channels:** Dynamically creates per-process Named Pipes (`FIFOs` keyed by client PID) so concurrent clients receive responses without cross-talk.
- **Zombie-Free Process Lifecycle:** Implements custom POSIX signal handlers (`SIGINT`, `SIGCHLD`) to reap child processes cleanly and unlink shared memory and semaphore descriptors on shutdown.

## Tech Stack

- **Language:** C (C11)
- **OS & Kernel APIs:** Linux, POSIX Signals (`SIGINT`, `SIGCHLD`), System V Shared Memory (`shmget`/`shmat`), Semaphores (`semop`), Named Pipes (`mkfifo`), `fork()`

## Build & Run

```bash
# 1. Compile the server and client binaries
make

# 2. Start the server in the background
./server &

# 3. Run the client to request a file
./cliente
```
