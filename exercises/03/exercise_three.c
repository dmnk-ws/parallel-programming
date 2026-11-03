#include <stdlib.h>
#include <stdio.h>
#include <omp.h> // needs -fopenmp: without it the pragma is ignored and the omp_* calls fail to link

int main(int argc, char** argv) {
    // without braces only the next statement would run in every thread
    #pragma omp parallel
    {
        printf("I am parallel     (thread %d of %d)\n", omp_get_thread_num(), omp_get_num_threads());
        printf("I am not parallel (thread %d of %d)\n", omp_get_thread_num(), omp_get_num_threads());
    }

    printf("outside the parallel region: %d thread(s)\n", omp_get_num_threads());

    return 0;
}
