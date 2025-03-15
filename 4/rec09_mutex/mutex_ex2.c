#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#define NUM_THREADS 15
int counter = 0;
mtx_t lock;

//====================================================
int next_counter(void *t) {
  long tid = (long)t;
  int rc;

  printf("Thread %ld starting...\n", tid);

  for (int i = 0; i < 10000000; ++i) {
    rc = mtx_lock(&lock);
    if (rc != thrd_success) {
      printf("ERROR in mtx_lock(): "
             "\n");
      exit(-1);
    }

    register int t = counter;
    ++t;
    counter = t;
    //++counter;

    rc = mtx_unlock(&lock);
    if (rc != thrd_success) {
      printf("ERROR in mtx_unlock()\n");
      exit(-1);
    }
  }
  rc = mtx_lock(&lock);
  printf("Thread %ld done. Result = %d\n", tid, counter);
  rc = mtx_unlock(&lock);
  thrd_exit((int)tid);
}

//====================================================
int main(int argc, char *argv[]) {
  thrd_t thread[NUM_THREADS];
  int rc;
  int status;

  // --- Initialize mutex ---------------------------
  rc = mtx_init(&lock, mtx_plain);
  if (rc != thrd_success) {
    printf("ERROR in mtx_init()\n");
    exit(-1);
  }

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
      printf("ERROR in thrd_join()\n");
      exit(-1);
    }
    printf("Main: completed join with thread %ld "
           "having a status of %d\n",
           t, status);
  }

  // ---  Epilogue -----------------------------------
  printf("Main: program completed. Exiting."
         "  Counter = %d\n",
         counter);

  mtx_destroy(&lock);
  thrd_exit(0);
}
//=================== END OF FILE ====================
