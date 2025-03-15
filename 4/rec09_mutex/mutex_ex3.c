#include <math.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#define NUM_THREADS 5
static long TOTAL;
// int counter = 1;
atomic_int counter = 1;
mtx_t lock;

//====================================================
int next_counter(void) {
  // mtx_lock( &lock );
  // register int temp = ++counter;
  // mtx_unlock( &lock );
  // return temp;

  return ++counter;
}

//====================================================
bool is_prime(long x) {
  for (long i = 2; i < x / 2; ++i) {
    if (0 == x % i)
      return false;
  }
  return true;
}

//====================================================
int prime_print(void *t) {
  unsigned int i = 0;
  long j = 0L;
  long tid = (long)t;

  printf("Thread %ld starting...\n", tid);

  while (j < TOTAL) {
    j = next_counter();
    ++i;
    if (is_prime(j)) {
      printf("Prime: %ld\n", j);
    }
  }

  printf("Thread %ld done.\n", tid);
  thrd_exit(i);
}

//====================================================
int main(int argc, char *argv[]) {
  thrd_t thread[NUM_THREADS];
  int rc;
  int status;

  TOTAL = pow(10, 5);

  // --- Initialize mutex ----------------------------
  rc = mtx_init(&lock, mtx_plain);
  if (rc != thrd_success) {
    printf("ERROR in mtx_init()\n");
    exit(-1);
  }

  // --- Launch threads ------------------------------
  for (long t = 0; t < NUM_THREADS; ++t) {
    printf("Main: creating thread %ld\n", t);
    rc = thrd_create(&thread[t], prime_print, (void *)t);
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

  // --- Epilogue -------------------------------------
  printf("Main: program completed. Exiting."
         " Counter = %d\n",
         counter);

  mtx_destroy(&lock);
  thrd_exit(0);
}
//=================== END OF FILE ====================
