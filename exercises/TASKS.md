# Exercise Sheet 01

## Task 2

Approximates pi with the Leibniz series pi = 4*(1 - 1/3 + 1/5 - 1/7 + ...).

```sh
cc -o pi pi.c -lm
time ./pi 1000000000
```

### Accuracy vs. iterations

`pi.c` prints with `%lf` (6 decimals), so the digits below come from a run printing 15 decimals.

| Iterations n  | Result              | Correct decimals | `time` total |
|---------------|---------------------|------------------|--------------|
| 10            | 3.041839618929403   | 0                | 0.444 s*     |
| 100           | 3.131592903558554   | 1                | 0.002 s      |
| 1 000         | 3.140592653839794   | 2                | 0.003 s      |
| 10 000        | 3.141492653590034   | 3                | 0.003 s      |
| 100 000       | 3.141582653589720   | 4                | 0.003 s      |
| 1 000 000     | 3.141591653589774   | 5                | 0.007 s      |
| 10 000 000    | 3.141592553589792   | 6                | 0.047 s      |
| 100 000 000   | 3.141592643589326   | 7                | 0.410 s      |
| 1 000 000 000 | 3.141592652588050   | 8                | 4.081 s      |
| true pi       | 3.141592653589793   |                  |              |

\* First run includes one-off startup cost (fresh binary, cold caches).

- The error is = 1/n: each 10x more iterations gives about one more correct decimal.
- Runtime grows linearly with n.
- With 6-decimal output, the result reads `3.141593` from n = 10^7 on, which hides further improvement.
- Only a single digit is off; the digits after it match pi again, because the error is almost exactly 1/n.

### a) Measuring with `time`

zsh (macOS), n = 10^9:

```
./pi 1000000000  4,05s user 0,03s system 99% cpu 4,081 total
```

bash format:

```
real    0m4.081s
user    0m4.050s
sys     0m0.030s
```

### b) Meaning of the time values

- **real / total**: wall-clock time from start to finish, including waiting for I/O, other processes and program loading.
- **user**: CPU time spent running the program's own code in user mode (the loop and `pow()`).
- **sys / system**: CPU time the kernel spent on behalf of the program (loading, memory allocation, the `write` syscall behind `printf`).
- **cpu %** (zsh only): (user + sys) / real.

A single-threaded program has user + sys <= real. A multithreaded program can have user + sys > real, because CPU time is summed across all cores while wall-clock time goes down.

## Task 3

`pi_threads.c` computes the same Leibniz series with POSIX threads.

```sh
cc -O2 -pthread -o pi_threads pi_threads.c
./pi_threads <block|cyclic|mutex> <n> <T>
```

`main` allocates one `pthread_t` and one `Work` struct per thread. A `Work` struct describes the indices a thread handles (`start`, `end`, `step`) and has a `sum` field for its result. The selected variant creates T threads with `pthread_create`, waits for them with `pthread_join` and combines the partial results. The result is multiplied by 4 at the end.

In `block` and `cyclic`, each thread sums into a local variable and writes `sum` in its own struct exactly once at the end. No two threads write the same memory, so no lock is needed. `pthread_join` guarantees that `main` sees the finished values.

### Variants

Example with n = 10, T = 3:

**block**: contiguous chunks with step 1. The last thread also takes the remainder.

```
k:       0  1  2 | 3  4  5 | 6  7  8  9
thread:  0  0  0 | 1  1  1 | 2  2  2  2
```

**cyclic**: thread i starts at k = i and steps by T up to n.

```
k:       0  1  2  3  4  5  6  7  8  9
thread:  0  1  2  0  1  2  0  1  2  0
```

**mutex**: same split as cyclic, but every single term is added to one global `sum` between `pthread_mutex_lock` and `pthread_mutex_unlock`. This is the deliberately bad variant, to show the cost of synchronization.

### Runtime vs. n and T

Apple M1 Pro (6 performance + 2 efficiency cores). Both programs are built with `-O2`, and `pi.c` uses the same `k % 2` sign as the threads instead of `pow`. `bench.sh` measures every combination with `/usr/bin/time -p` (3 runs, median of `real`) and writes `results.csv`. Speedup S = t_seq / t_T, efficiency E = S / T.

**block**, `real` in s (cyclic is within ±0.03 s):

| n     | seq  | T=1  | T=2  | T=4  | T=6  | T=8  | T=16 | T=32 |
|-------|------|------|------|------|------|------|------|------|
| 10^6  | 0.00 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00 |
| 10^7  | 0.01 | 0.01 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00 |
| 10^8  | 0.09 | 0.09 | 0.05 | 0.02 | 0.01 | 0.01 | 0.01 | 0.01 |
| 10^9  | 0.94 | 0.96 | 0.49 | 0.25 | 0.17 | 0.14 | 0.14 | 0.14 |
| 10^10 | 9.45 | 9.57 | 4.93 | 2.53 | 1.69 | 1.41 | 1.41 | 1.40 |
| S (10^10) | 1.00 | 0.99 | 1.92 | 3.74 | 5.59 | 6.70 | 6.70 | 6.75 |
| E (10^10) | 1.00 | 0.99 | 0.96 | 0.93 | 0.93 | 0.84 | 0.42 | 0.21 |

### Comparison of the three variants

|                                  | block                       | cyclic                      | mutex                              |
|----------------------------------|-----------------------------|-----------------------------|------------------------------------|
| Work split                       | contiguous chunks, step 1   | k = i, i+T, i+2T, …         | like cyclic                        |
| Combining results                | local sum, once per thread  | local sum, once per thread  | shared sum, lock per term          |
| Synchronization                  | only `pthread_join`         | only `pthread_join`         | 2 × n lock operations              |
| n = 10^8, T = 1                  | 0.09 s                      | 0.09 s                      | 0.84 s                             |
| n = 10^8, T = 8                  | 0.01 s                      | 0.01 s                      | 1.88 s                             |
| n = 10^10, T = 8 (S)             | 1.41 s (6.70)               | 1.44 s (6.56)               | not measured (hours)               |
| Scaling with T                   | near-linear up to 6, max 8  | near-linear up to 6, max 8  | gets slower, always < sequential   |
| `sys` time                       | ≈ 0                         | ≈ 0                         | up to 12 s (kernel sleep/wake)     |

- **block and cyclic perform the same here.** Every term costs the same, so both splits give every thread the same amount of work. Block is the simpler one to write. Cyclic would only win if the cost per term varied with k.
- **mutex is correct but unusable.** It gives the same result, but the per-term lock costs more than the computation itself. With several threads, they mostly wait for each other.
- **Takeaway:** compute locally and combine once at the end. Synchronizing inside the hot loop destroys any speedup.

### Observations

- **Thread count:** near-linear scaling up to 6 threads (E ≈ 0.93), one thread per performance core. From 6 to 8 threads, S only rises from 5.59 to 6.70 instead of about 7.5, because the last two threads run on the slower efficiency cores. More than 8 threads doesn't help, because there are only 8 cores.
- **Small n:** at n ≤ 10^7, every run takes less than the 0.01 s resolution of `time`, so threads bring nothing measurable, and the point where thread creation stops costing more than it saves can't be measured this way. Only n ≥ 10^9 gives reliable speedups.
- **Mutex:** already 9× slower than `pi.c` with 1 thread (0.84 s vs. 0.09 s at n = 10^8), because one lock and unlock per term costs more than the term itself. With more threads, they mostly wait for each other, and `sys` time grows to 12 s while the kernel puts threads to sleep and wakes them.

### Pros and cons of pthreads

**Pros**

- Fine-grained control over the number of threads, the work split and the synchronization.
- Shared memory: threads read the same data without copying.
- POSIX standard, available on practically every Unix system, with little overhead on top of OS threads.

**Cons**

- Everything by hand: work split, `void *` arguments, create/join, combining results. OpenMP does the same with `#pragma omp parallel for reduction(+:sum)`.
- Easy to get wrong: race conditions without a lock, over-synchronization with a lock (see `mutex`), false sharing.
- No built-in reduction or scheduling.
- Thread creation has a cost, which doesn't pay off for small problems.
- Only scales within one machine (shared memory).

<br>
<br>

# Exercise Sheet 02

## Task 1

### What happens

```
$ ./hello
Hello World!        (8 times)

$ OMP_NUM_THREADS=3 ./hello
Hello World!        (3 times)

$ gcc -o hello hello.c        # without -fopenmp
warning: ignoring '#pragma omp parallel' [-Wunknown-pragmas]
$ ./hello
Hello World!        (once)
```

### Why

- `#pragma omp parallel` opens a **parallel region**. The main thread becomes the master of a team of threads (fork), every thread executes the block `{ ... }` once, and at the closing brace all threads wait at an implicit barrier and the team ends (join). Only the master continues to `return 0`.
- No `num_threads` clause and no `OMP_NUM_THREADS` is set, so the runtime (`libgomp`) uses its default: one thread per logical core, here 8. That's why the line appears 8 times.
- The block contains no work split, so every thread does the same thing. `#pragma omp parallel` alone only replicates the code. Splitting work needs a worksharing construct such as `for`, or the thread ID from `omp_get_thread_num()`.
- The threads run concurrently, so the order of the output is not defined. Here every line is the same, so it can't be seen; with the thread ID printed, the order would change between runs. Each `printf` call is thread-safe, so lines don't get mixed up character by character.
- `-fopenmp` makes the compiler translate the pragma into calls to the OpenMP runtime and links `libgomp`. Without it, the pragma is ignored (only a warning) and the program runs sequentially with one output line. The same source works both ways.

<br>
<br>

# Exercise Sheet 03

## Task 2

Tseq = 100 s, measured runtimes in `task2.csv`. Every value is computed by a gnuplot script (one per subtask, named after it) and collected in `task2.xlsx`.

### 1. Speedup and efficiency

S(p) = Tseq / Tpar(p), E(p) = S(p) / p (`speedup_2-1.gp`, `efficiency_2-1.gp`):

| p | 1 | 2 | 4 | 8 | 16 |
|---|---|---|---|---|---|
| Tpar(p) [s] | 100 | 55 | 32 | 20 | 14 |
| S(p) | 1.00 | 1.82 | 3.13 | 5.00 | 7.14 |
| E(p) | 1.00 | 0.91 | 0.78 | 0.63 | 0.45 |

The speedup keeps growing, but further and further below the ideal S = p. The efficiency falls steadily; at p = 16 less than half of the processor time goes into useful work.

### 2. Overhead

O(p) = p · Tpar(p) − Tseq (`overhead_2-2.gp`):

| p | 1 | 2 | 4 | 8 | 16 |
|---|---|---|---|---|---|
| p · Tpar(p) [s] | 100 | 110 | 128 | 160 | 224 |
| O(p) [s] | 0 | 10 | 28 | 60 | 124 |
| O / (p · Tpar) | 0 % | 9 % | 22 % | 38 % | 55 % |

- From p = 4 on, the overhead roughly doubles with every doubling of p (2 → 4 is ×2.8), so it grows about linearly: each additional processor adds about 8 s of total processor time that does no useful work.
- The useful work stays at 100 s, so the overhead's share of the total processor time rises (this share is exactly 1 − E). From p = 16 on, more processor time goes into overhead than into the actual computation.
- Linear growth is what a constant sequential part causes: while one processor runs it, the other p − 1 wait, so O(p) ≈ (p − 1) · f · Tseq. With f from 2.3 this gives 15 · 8.27 s ≈ 124 s at p = 16.
- Consequence: every further processor brings less speedup but costs the same amount of overhead, so adding processors pays off less and less.

### 3. Sequential fraction (Amdahl)

Solving S(p) = 1 / (f + (1 − f) / p) for f gives f = (1/S − 1/p) / (1 − 1/p). With S(16) = 100/14 ≈ 7.14 (`amdahl_fit_2-3.gp`):

- **f ≈ 0.083** (8.3 % sequential)
- **Upper bound** for p → ∞: S(∞) = 1/f ≈ **12.1**. At p = 16 the program already reaches about 60 % of that limit.

Computed from each measurement on its own, f is not constant:

| p | 2 | 4 | 8 | 16 |
|---|---|---|---|---|
| f | 0.100 | 0.093 | 0.086 | 0.083 |

f can't be determined from p = 1, because S(1) = 1 for every f (0/0). The f fitted at p = 16 predicts slightly more speedup at p = 2, 4 and 8 than was measured, so a single fixed sequential fraction doesn't describe the program exactly. Part of the losses appears as soon as the program runs in parallel at all (e.g. starting threads, distributing work). f ≈ 0.083 is therefore a model parameter fitted to one measurement, and the bound of about 12 is an estimate, not a guarantee.

### 4. Gustafson's Law

With the sequential fraction f ≈ 0.083 from 2.3 and p = 16 (`gustafson_2-4.gp`):

| | S(16) | Limit for p → ∞ |
|---|---|---|
| Amdahl (strong scaling) | 7.14 | 1/f ≈ 12.1 |
| Gustafson (weak scaling): S(p) = f + (1 − f) · p | 14.76 | none, grows linearly with slope 1 − f ≈ 0.92 |

Amdahl keeps the problem size fixed, so with more processors only the parallel part gets faster, the sequential part dominates more and more, and the speedup is limited to 1/f. Gustafson instead scales the problem with p, because in practice more processors are used to solve larger problems in the same time. The sequential part stays constant in absolute time while the parallel work grows, so its share f of the total work shrinks and the speedup keeps growing almost linearly without an upper bound. This only holds if the problem can actually be scaled up (it is meaningful and fits in memory) and the sequential part does not grow with the problem size, or grows more slowly than the parallel part.

Formula from J. L. Gustafson, "Reevaluating Amdahl's Law", Communications of the ACM 31(5), 1988. Strictly, Gustafson's f is the sequential share of the parallel run's time; using the f from Amdahl is a simplification.

## Task 3

```sh
gcc -fopenmp -o exercise_three exercise_three.c
OMP_NUM_THREADS=2 ./exercise_three
```

### 1. What the program does

- `#pragma omp parallel` applies to the **next statement** (the structured block after it). Without braces, that is only the first `printf`.
- The master thread starts a team of threads (fork). Every thread prints "I am parallel" once. At the end of the region all threads wait at the implicit barrier and the team ends (join).
- Only the master continues and prints "I am not parallel" exactly once. Because of the barrier, it is always the last line.
- Without `OMP_NUM_THREADS`, the runtime uses one thread per logical core: 8 on the M1 Pro, so 8 × "I am parallel" and 1 × "I am not parallel".
- "I am parallel" is not parallel *work*: every thread does the same thing, the code is only replicated.

### 2./3. Compiling and running

| `OMP_NUM_THREADS` | "I am parallel" | "I am not parallel" |
|---|---|---|
| 1 | 1 | 1 |
| 2 | 2 | 1 |
| 4 | 4 | 1 |
| not set | 8 | 1 |

- `OMP_NUM_THREADS=2 ./exercise_three` sets the variable for this one command only; `export OMP_NUM_THREADS=2` sets it for every following command in the shell.
- Without `-fopenmp`, the pragma is ignored (a warning only with `-Wall`) and the program runs sequentially: each line once.

### 4. Both statements in all threads

Put both `printf` calls into a block `{ ... }` after the pragma. With 2 threads there are now 4 lines. Each `printf` is printed in one piece, but the threads can interleave between the two calls, so the order (e.g. P N P N or P P N N) changes between runs.

### 5. Thread ID and number of threads

`omp_get_thread_num()` returns the ID of the calling thread in its team (0 … n−1), `omp_get_num_threads()` the size of the current team. With `OMP_NUM_THREADS=4`, two runs:

```
I am parallel     (thread 0 of 4)       I am parallel     (thread 1 of 4)
I am not parallel (thread 0 of 4)       I am not parallel (thread 1 of 4)
I am parallel     (thread 1 of 4)       I am parallel     (thread 0 of 4)
I am not parallel (thread 1 of 4)       I am not parallel (thread 0 of 4)
I am parallel     (thread 3 of 4)       I am parallel     (thread 3 of 4)
I am not parallel (thread 3 of 4)       I am not parallel (thread 3 of 4)
I am parallel     (thread 2 of 4)       I am parallel     (thread 2 of 4)
I am not parallel (thread 2 of 4)       I am not parallel (thread 2 of 4)
outside the parallel region: 1 thread(s)
```

Observations:

- The order of the thread IDs changes between runs: which thread runs first is decided by the OS scheduler, not by the ID.
- Inside the region every thread reports 4, the size of its team.
- Outside the region `omp_get_num_threads()` returns **1**, although `OMP_NUM_THREADS=4` is still set. After the join only the master is left, so the current team has one thread. The function reports the team that is running, not the configured number.
- `omp_get_max_threads()` at the same place returns 4: the number of threads the *next* parallel region would get.
- Unlike the original program, this version no longer builds without `-fopenmp`: it calls OpenMP functions, so the linker reports undefined symbols for them.

## Task 4

```
          A
        / | \
       B  C  D
        \ /   |
         E    F
          \  /
           G
           |
           H
```

### 1. Sequential work

Every node costs 1 time unit and one processor has to execute all of them: **Tseq = 8**.

### 2. Critical path

Three paths have the maximum length, **T∞ = 5**:

- A → B → E → G → H
- A → C → E → G → H
- A → D → F → G → H

Even with infinitely many processors the program can't finish in less than 5 steps, because the nodes on these paths depend on each other and must run one after another.

### 3. Degree of parallelism

**P = Tseq / T∞ = 8 / 5 = 1.6**

- P is the average number of operations that can run at the same time, and an upper bound for the speedup: S(p) ≤ 1.6 for any number of processors.
- The maximum width of the graph is 3 (B, C, D). With 3 processors T∞ = 5 is reached and S = 1.6, but most of the time processors are idle: E(3) = 1.6 / 3 ≈ 0.53.

### 4. Schedule on 2 processors

| Step | P1 | P2 |
|---|---|---|
| 1 | A | – |
| 2 | B | C |
| 3 | E | D |
| 4 | F | – |
| 5 | G | – |
| 6 | H | – |

**Tpar(2) = 6**, **S(2) = 8 / 6 ≈ 1.33**, E(2) ≈ 0.67.

The schedule is optimal. The lower bounds are Tpar ≥ T∞ = 5 and Tpar ≥ ⌈Tseq / p⌉ = 4, so 5 would be the best case. But B, C and D all lie on a critical path and have no slack: finishing in 5 steps would require all three to run in step 2, which needs 3 processors. With 2, one of them is delayed by a step and its critical path becomes 6 long. Running B and D first and delaying C instead (and therefore E) gives the same Tpar = 6.
