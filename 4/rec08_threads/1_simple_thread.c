#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

int thread_func(void *thread_param) {
  // thrd_current type is implementation defined
  printf("In thread #%ld\n", thrd_current());
  printf("I received \"%s\" from my caller\n", (char *)thread_param);

  thrd_exit(EXIT_SUCCESS);
  // return EXIT_SUCCESS;// <- same as this
}

int main(int argc, char *argv[]) {
  printf("In main thread #%lu\n", thrd_current());

  thrd_t thread_id;
  int rc = thrd_create(&thread_id, thread_func, (void *)"hello");
  if (rc != thrd_success) {
    // The value of thrd_success is implementation defined, it doesn't have to
    // be 0
    fprintf(stderr, "Failed creating thread\n");
    exit(EXIT_FAILURE);
  }

  // thrd_exit(EXIT_SUCCESS);
  return EXIT_SUCCESS; //<- NOT the same as this
}
