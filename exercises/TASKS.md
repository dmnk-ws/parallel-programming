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
