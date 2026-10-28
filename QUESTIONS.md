# Questions

## Lecture 2

### 1. On a shared-memory system, how do the threads of a program exchange data?

Implicitly, by reading and writing shared data

All threads see the same address space: one thread writes a variable, another reads it — no explicit sending needed. The flip side is that concurrent access must be synchronized to avoid race conditions. Exchanging data by messages is the model of distributed-memory systems (e.g. MPI on a cluster).

### 2. In a hybrid MPI + OpenMP program on a cluster of multicore nodes, what is each model typically used for?

OpenMP inside a node, MPI between the nodes

The cores within a node share memory, which suits OpenMP's threads. Separate nodes have no shared memory and can only communicate over the network, which is what MPI's message passing is for. MPI would also work inside a node, but OpenMP cannot reach across nodes.

### 3. Which of these is shared by all threads of one process?

The heap

Threads of a process share one address space: heap, global variables and program code. Each thread has its own stack (own function calls and local variables), program counter (own position in the code) and register set (own execution state, saved and restored on context switches). Because the heap is shared, it is where race conditions arise.

### 4. In an SPMD program all processes run the same program. How can they still do different things?

The code branches on the ID (rank) of the process

SPMD = Single Program, Multiple Data. All processes start from the same executable, but each gets its own rank and uses it to pick its share of the data (e.g. which block of an array) or a role (typically rank 0 as coordinator). Starting different executables would be MPMD (Multiple Program, Multiple Data).

### 5. Two threads take turns on a single core (time slicing). How would you call this?

Concurrent, but not parallel

Concurrency means several tasks are in progress during the same period of time; parallelism means they actually execute at the same instant, which requires multiple cores. With time slicing on one core, the threads interleave — both make progress, but at any moment only one runs. Parallelism implies concurrency, not the other way around.
