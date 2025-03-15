/*
 *  Based on:
 *  https://computing.llnl.gov/tutorials/pthreads/#ConditionVariables
 *
 * */
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

#define TCOUNT 10
#define COUNT_LIMIT 12

int count = 0;
const long thread_ids[] = {0, 1, 2};
const int NUM_THREADS = sizeof thread_ids / sizeof(long);

mtx_t count_mutex;
cnd_t count_threshold_cv;

//----------------------------------------------------------------------------
int inc_count(void *t) {
  long my_id = (long)t;

  for (int i = 0; i < TCOUNT; i++) {
    mtx_lock(&count_mutex);
    count++;

    // Check the value of count and signal waiting thread when condition is
    // reached.  Note that this occurs while mutex is locked.
    if (count == COUNT_LIMIT) {
      cnd_signal(&count_threshold_cv);
      printf("inc_count(): thread %ld, count = %d  Threshold reached.\n", my_id,
             count);
    }

    printf("inc_count(): thread %ld, count = %d, unlocking mutex\n", my_id,
           count);
    mtx_unlock(&count_mutex);

    // Do some "work" so threads can alternate on mutex lock
    thrd_sleep(&(struct timespec){.tv_sec = 1}, NULL); // sleep 1 sec
  }
  thrd_exit(0);
}

//----------------------------------------------------------------------------
int watch_count(void *t) {
  long my_id = (long)t;

  printf("Starting watch_count(): thread %ld\n", my_id);

  /*
   Lock mutex and wait for signal.  Note that the cnd_wait
   routine will automatically and atomically unlock mutex while it waits.
   Also, note that if COUNT_LIMIT is reached before this routine is run by
   the waiting thread, the loop will be skipped to prevent cnd_wait
   from never returning.
  */
  mtx_lock(&count_mutex);
  while (count < COUNT_LIMIT) {
    printf("watch_count(): thread %ld Meditating on condition variable.\n",
           my_id);
    cnd_wait(&count_threshold_cv, &count_mutex);
    printf("watch_count(): thread %ld Condition signal received.\n", my_id);

    count += 125;

    printf("watch_count(): thread %ld count now = %d.\n", my_id, count);
  }
  mtx_unlock(&count_mutex);
  thrd_exit(0);
}

//----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
  thrd_t threads[NUM_THREADS];

  // Initialize mutex and condition variable objects
  mtx_init(&count_mutex, mtx_plain);
  cnd_init(&count_threshold_cv);

  thrd_create(&threads[0], watch_count, (void *)thread_ids[0]);
  for (int i = 1; i < NUM_THREADS; ++i) {
    thrd_create(&threads[i], inc_count, (void *)thread_ids[i]);
  }

  // Wait for all threads to complete
  for (int i = 0; i < NUM_THREADS; i++) {
    thrd_join(threads[i], NULL);
  }

  printf("Main(): Waited on %d  threads. Done.\n", NUM_THREADS);

  // Clean up and exit
  mtx_destroy(&count_mutex);
  cnd_destroy(&count_threshold_cv);
  thrd_exit(0);
}
//============================== END OF FILE =================================
