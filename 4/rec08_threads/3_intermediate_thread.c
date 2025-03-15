#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

int thread_func(void *thread_param) {
  int *y = (int *)thread_param;
  printf("In thread #%ld\n", thrd_current());
  printf("I received %d from my caller\n", *y);

  *y += 1;

  return *y;
}

int main(int argc, char *argv[]) {
  printf("In main thread #%lu\n", thrd_current());

  thrd_t thread_id;
  int *x = malloc(sizeof(int));
  *x = 5;

  int rc = thrd_create(&thread_id, thread_func, (void *)x);
  if (rc != thrd_success) {
    fprintf(stderr, "Failed creating thread:\n");
    free(x);
    exit(EXIT_FAILURE);
  }

  int thread_return;
  thrd_join(thread_id, &thread_return);

  printf("Thread %lu finished and returned %d\n", thread_id, thread_return);
  printf("Original value was %d\n", *x);

  free(x);

  return EXIT_SUCCESS;
}
