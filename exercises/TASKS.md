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

`main` allocates one `pthread_t` and one `Work` struct per thread. A `Work` struct describes the
indices a thread handles (`start`, `end`, `step`) and has a `sum` field for its result.
The selected variant creates T threads with `pthread_create`, waits for them with `pthread_join`
and combines the partial results. The result is multiplied by 4 at the end.

In `block` and `cyclic`, each thread sums into a local variable and writes `sum` in its own struct
exactly once at the end. No two threads write the same memory, so no lock is needed.
`pthread_join` guarantees that `main` sees the finished values.

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

**mutex**: same split as cyclic, but every single term is added to one global `sum`
between `pthread_mutex_lock` and `pthread_mutex_unlock`. This is the deliberately bad variant,
to show the cost of synchronization.

### Runtime vs. n and T

Apple M1 Pro (6 performance + 2 efficiency cores). Both programs are built with `-O2`, and `pi.c`
uses the same `k % 2` sign as the threads instead of `pow`. `bench.sh` measures every combination
with `/usr/bin/time -p` (3 runs, median of `real`) and writes `results.csv`.
Speedup S = t_seq / t_T, efficiency E = S / T.

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

- **block and cyclic perform the same here.** Every term costs the same, so both splits give
  every thread the same amount of work. Block is the simpler one to write. Cyclic would only win
  if the cost per term varied with k.
- **mutex is correct but unusable.** It gives the same result, but the per-term lock costs more
  than the computation itself. With several threads, they mostly wait for each other.
- **Takeaway:** compute locally and combine once at the end. Synchronizing inside the hot loop
  destroys any speedup.

### Observations

- **Thread count:** near-linear scaling up to 6 threads (E ≈ 0.93), one thread per performance
  core. From 6 to 8 threads, S only rises from 5.59 to 6.70 instead of about 7.5, because the last
  two threads run on the slower efficiency cores. More than 8 threads doesn't help, because there
  are only 8 cores.
- **Small n:** at n ≤ 10^7, every run takes less than the 0.01 s resolution of `time`, so threads
  bring nothing measurable, and the point where thread creation stops costing more than it saves
  can't be measured this way. Only n ≥ 10^9 gives reliable speedups.
- **Mutex:** already 9× slower than `pi.c` with 1 thread (0.84 s vs. 0.09 s at n = 10^8), because
  one lock and unlock per term costs more than the term itself. With more threads, they mostly wait
  for each other, and `sys` time grows to 12 s while the kernel puts threads to sleep and wakes them.

### Pros and cons of pthreads

**Pros**

- Fine-grained control over the number of threads, the work split and the synchronization.
- Shared memory: threads read the same data without copying.
- POSIX standard, available on practically every Unix system, with little overhead on top of
  OS threads.

**Cons**

- Everything by hand: work split, `void *` arguments, create/join, combining results.
  OpenMP does the same with `#pragma omp parallel for reduction(+:sum)`.
- Easy to get wrong: race conditions without a lock, over-synchronization with a lock
  (see `mutex`), false sharing.
- No built-in reduction or scheduling.
- Thread creation has a cost, which doesn't pay off for small problems.
- Only scales within one machine (shared memory).
