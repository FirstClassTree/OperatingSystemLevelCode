#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

int thread_func(void *toWait) {
	thrd_t *t = (thrd_t*) toWait;
	int ret;
	thrd_join(*t, &ret);

	printf("I received \"%d\" from my join\n", ret);

	// thrd_exit(EXIT_SUCCESS);
	return EXIT_SUCCESS;// <- same as this
}

int main(int argc, char *argv[]) {
  printf("In main thread #%lu\n", thrd_current());

  thrd_t thread_id, self;
  self = thrd_current();
  thrd_create(&thread_id, thread_func, (void*)&self);
  thrd_exit(125);
}
