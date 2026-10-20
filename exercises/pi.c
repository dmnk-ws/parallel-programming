#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Please provide the number of iterations");
    return 1;
  }
  
  long max = strtol(argv[1], NULL, 10);
  double pi = 0.0;
  
  for (long k = 0; k < max; k++) {
    pi += pow(-1,k) / (2 * k + 1);
  }
  pi *= 4;

  printf("The approximation of pi is: %lf\n", pi);

  return 0;
}

