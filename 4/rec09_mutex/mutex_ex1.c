#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#define NUM_THREADS 15
atomic_int counter = 0;

//====================================================
int next_counter(void *t) {
  long tid = (long)t;

  printf("Thread %ld starting...\n", tid);

  for (int i = 0; i < 10000000; ++i) {
    // register int r = counter;
    //++r;
    // counter = r;
    ++counter;
  }
  printf("Thread %ld done. Result = %d\n", tid, counter);
  thrd_exit((int)tid);
}

//====================================================
int main(int argc, char **argv) {
  thrd_t thread[NUM_THREADS];
  int rc;
  int status;

  // --- Launch threads ------------------------------
  for (long t = 0; t < NUM_THREADS; ++t) {
    printf("Main: creating thread %ld\n", t);
    rc = thrd_create(&thread[t], next_counter, (void *)t);
    if (rc != thrd_success) {
      printf("ERROR in thrd_create()\n");
      exit(-1);
    }
  }

  // --- Wait for threads to finish ------------------
  for (long t = 0; t < NUM_THREADS; ++t) {
    rc = thrd_join(thread[t], &status);
    if (rc != thrd_success) {
      printf("ERROR in thrd_join():\n");
      exit(-1);
    }
    printf("Main: completed join with thread %ld "
           "having a status of %d\n",
           t, status);
  }

  // --- Epilogue ------------------------------------
  printf("Main: program completed. Exiting."
         " Counter = %d\n",
         counter);
  thrd_exit(0);
}
//=================== END OF FILE ====================
