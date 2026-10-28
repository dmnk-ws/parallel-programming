#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

int count3s_sequential(int *a, int n) {
    int count = 0;

    for (int i = 0; i < n; ++i) {
        if (a[i] == 3) {
            ++count;
        }
    }

    return count;
}

int count3s_v1(int *a, int n, int T) {
    int count = 0;
    omp_set_num_threads(T);

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; ++i) {
        if (a[i] == 3) {
            ++count;
        }
    }

    return count;
}

int count3s_v2(int *a, int n, int T) {
    int count = 0;
    omp_set_num_threads(T);

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; ++i) {
        if (a[i] == 3) {
            #pragma omp atomic
            ++count;
        }
    }

    return count;
}

int count3s_v3(int *a, int n, int T) {
    int count = 0;
    int private_count[T];

    for (int i = 0; i < T; ++i) {
        private_count[i] = 0;
    }

    omp_set_num_threads(T);

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; ++i) {
        if (a[i] == 3) {
            ++private_count[omp_get_thread_num()];
        }
    }

    #pragma omp parallel
    {
        #pragma omp atomic
        count += private_count[omp_get_thread_num()];
    }

    return count;
}

int count3s_v4(int *a, int n, int T) {
    int count = 0;

    struct padded_int {
        int value;
        // performance portability, due to 128bytes
        char padding[124];
    } private_count[T];

    for (int i = 0; i < T; ++i) {
        private_count[i].value = 0;
    }

    omp_set_num_threads(T);

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; ++i) {
        if (a[i] == 3) {
            ++private_count[omp_get_thread_num()].value;
        }
    }

    #pragma omp parallel
    {
        #pragma omp atomic
        count += private_count[omp_get_thread_num()].value;
    }

    return count;
}

int *init(int n) {
    int *arr = malloc(n * sizeof(int));

    srand(time(NULL));
    for (int i = 0; i < n; ++i) {
       int value = rand() % 10;
       arr[i] = value;
    }

    return arr;
}

void print_result(const char *name, int count, double time) {
    printf("%-10s %10d %12.6f\n", name, count, time);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("%s <n> <threads>\n", argv[0]);

        return 1;
    }

    int n = strtol(argv[1], NULL, 10);
    int T = strtol(argv[2], NULL, 10);

    if (n <= 0) {
        printf("Number of n must be positive\n");

        return 1;
    }

    if (T <= 0) {
        printf("Number of threads must be positive\n");

        return 1;
    }

    int *arr = init(n);
    int count;
    double start;

    printf("n = %d, threads = %d\n\n", n, T);
    printf("%-10s %10s %12s\n", "variant", "count", "time [s]");

    start = omp_get_wtime();
    count = count3s_sequential(arr, n);
    print_result("sequential", count, omp_get_wtime() - start);

    start = omp_get_wtime();
    count = count3s_v1(arr, n, T);
    print_result("v1", count, omp_get_wtime() - start);

    start = omp_get_wtime();
    count = count3s_v2(arr, n, T);
    print_result("v2", count, omp_get_wtime() - start);

    start = omp_get_wtime();
    count = count3s_v3(arr, n, T);
    print_result("v3", count, omp_get_wtime() - start);

    start = omp_get_wtime();
    count = count3s_v4(arr, n, T);
    print_result("v4", count, omp_get_wtime() - start);

    free(arr);

    return 0;
}
