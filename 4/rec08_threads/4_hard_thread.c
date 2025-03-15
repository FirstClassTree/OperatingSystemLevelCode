#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#define THREAD_COUNT 10

int thread_func(void *thread_param) {
  long i = (long)thread_param;
  printf("I'm thread number %ld\n", i);
  return 0;
}

int main(int argc, char *argv[]) {
  thrd_t thread_ids[THREAD_COUNT];
  for (size_t i = 0; i < THREAD_COUNT; i++) {
    int rc = thrd_create(&thread_ids[i], thread_func, (void *)i);
    if (rc != thrd_success) {
      fprintf(stderr, "Failed creating thread\n");
      exit(EXIT_FAILURE);
    }
  }

  for (size_t i = 0; i < THREAD_COUNT; i++) {
    thrd_join(thread_ids[i], NULL);
  }
  //for (size_t i = 0; i < THREAD_COUNT; i++) {
  //  thread_func((void *)i);
  //}

  return EXIT_SUCCESS;
}
