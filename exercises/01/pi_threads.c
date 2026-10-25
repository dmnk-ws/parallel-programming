#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct {
  long start, end, step;
  double sum;
} Work;

void *worker(void *arg) {
  Work *w = arg;
  double localSum = 0.0;

  for (long k = w->start; k < w->end; k += w->step) {
    localSum += (k % 2 ? -1.0 : 1.0) / (2 * k + 1);
  }
  w->sum = localSum;

  return NULL;
}

double sum = 0.0;
pthread_mutex_t sumLock = PTHREAD_MUTEX_INITIALIZER;

void *mutexWorker(void *arg) {
  Work *w = arg;

  for (long k = w->start; k < w->end; k += w->step) {
    double term = (k % 2 ? -1.0 : 1.0) / (2 * k + 1);

    pthread_mutex_lock(&sumLock);
    sum += term;
    pthread_mutex_unlock(&sumLock);
  }

  return NULL;
}

/*
 * Block: contiguous chunks, step = 1, last thread takes the remainder.
 * n = 10, T = 3:
 *
 *   k:       0  1  2 | 3  4  5 | 6  7  8  9
 *   thread:  0  0  0 | 1  1  1 | 2  2  2  2
 */
double block(long n, int T, pthread_t *threads, Work *work) {
  long chunk = n / T;

  for (int i = 0; i < T; i++) {
    work[i].start = i * chunk;
    work[i].step = 1;
    work[i].end = (i == T - 1) ? n : (i + 1) * chunk;
    pthread_create(&threads[i], NULL, worker, &work[i]);
  }

  double pi = 0.0;
  for (int i = 0; i < T; i++) {
    pthread_join(threads[i], NULL);
    pi += work[i].sum;
  }

  pi *= 4;

  return pi;
}

/*
 * Cyclic: thread i starts at k = i, step = T, end = n.
 * n = 10, T = 3:
 *
 *   k:       0  1  2  3  4  5  6  7  8  9
 *   thread:  0  1  2  0  1  2  0  1  2  0
 */
double cyclic(long n, int T, pthread_t *threads, Work *work) {
  for (int i = 0; i < T; i++) {
    work[i].start = i;
    work[i].step = T;
    work[i].end = n;
    pthread_create(&threads[i], NULL, worker, &work[i]);
  }

  double pi = 0.0;
  for (int i = 0; i < T; i++) {
    pthread_join(threads[i], NULL);
    pi += work[i].sum;
  }

  pi *= 4;

  return pi;
}

/*
 * Mutex: cyclic split, but every term goes straight into the shared sum
 * under sumLock, so only one thread adds at a time.
 * n = 10, T = 3:
 *
 *   k:       0  1  2  3  4  5  6  7  8  9
 *   thread:  0  1  2  0  1  2  0  1  2  0
 *            |  |  |  |  |  |  |  |  |  |
 *            v  v  v  v  v  v  v  v  v  v
 *   sum:     lock -> += term -> unlock   (one thread at a time)
 */
double mutex(long n, int T, pthread_t *threads, Work *work) {
  sum = 0.0;
  for (int i = 0; i < T; i++) {
    work[i].start = i;
    work[i].step = T;
    work[i].end = n;
    pthread_create(&threads[i], NULL, mutexWorker, &work[i]);
  }

  for (int i = 0; i < T; i++) {
    pthread_join(threads[i], NULL);
  }

  double pi = sum * 4;

  return pi;
}

int main(int argc, char *argv[]) {
  if (argc < 4) {
    printf("%s <block|cyclic|mutex> <number of iterations> <number of threads>\n", argv[0]);

    return 1;
  }

  const char *type = argv[1];

  if (strcmp(type, "block") != 0 && strcmp(type, "cyclic") != 0 && strcmp(type, "mutex") != 0) {
    printf("Use <block|cyclic|mutex>\n");

    return 1;
  }

  double pi = 0.0;
  long n = strtol(argv[2], NULL, 10);
  int T = strtol(argv[3], NULL, 10);

  if (T <= 0) {
    printf("Number of threads must be positive\n");

    return 1;
  }

  pthread_t *threads = malloc(T * sizeof *threads);
  Work *work = malloc(T * sizeof *work);

  if (strcmp(type, "block") == 0) {
    pi = block(n, T, threads, work);
  } else if (strcmp(type, "cyclic") == 0) {
    pi = cyclic(n, T, threads, work);
  } else {
    pi = mutex(n, T, threads, work);
  }

  free(threads);
  free(work);

  printf("The approximation of pi is: %lf\n", pi);

  return 0;
}
