/******************************************************************************
 * FILE: condvar.c
 * DESCRIPTION:
 *   Example code for using Pthreads condition variables.  The main thread
 *   creates three threads.  Two of those threads increment a "count" variable,
 *   while the third thread watches the value of "count".  When "count"
 *   reaches a predefined limit, the waiting thread is signaled by one of the
 *   incrementing threads. The waiting thread "awakens" and then modifies
 *   count. The program continues until the incrementing threads reach
 *   TCOUNT. The main program prints the final value of count.
 * SOURCE: Adapted from example code in "Pthreads Programming", B. Nichols
 *   et al. O'Reilly and Associates.
 * LAST REVISED: 07/16/09  Blaise Barney
 ******************************************************************************/
#include <unistd.h>
#include <pthread.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>

#include <limits.h>    /* for CHAR_BIT */
#include <stdint.h>   /* for uint32_t */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* for threads */

struct thread_data {
    int thread_id;
    int max;
    unsigned long testnum;
};

/* for sieve */
typedef uint32_t word_t;
//enum {
//    BITS_PER_WORD = sizeof (word_t) * CHAR_BIT
//};

enum {
    BITS_PER_WORD = 16
};
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)

/* get bit at x*/
#define TEST(f,x)       *(f+WORD_OFFSET(x)) & (1 << (BIT_OFFSET(x)/2))
/* set bit at x */
#define SET(f,x)       *(f+WORD_OFFSET(x)) |= (1 << (BIT_OFFSET(x)/2))
/* clear */
#define CLEAR(f,x)       *(f+WORD_OFFSET(n)) &= ~(1 << BIT_OFFSET(n))

unsigned char *bitmap = NULL;
unsigned long testnum = 1, max, mom, hits = 1, count;

void *set(void *threadarg) {
    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;
    for (mom = 3 * my_data->testnum; mom < my_data->max; mom += testnum << 1)
        SET(bitmap, mom);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    time_t start;
    time_t theend;
    pthread_attr_t attr;
    //    long t1 = 1, t2 = 2, t3 = 3;
    //    pthread_t threads[3];

    (argc > 1) ? max = atol(argv[1]) : max = INT_MAX;

    /* For portability, explicitly create threads in a joinable state */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    bitmap = (unsigned char*) malloc(max >> 4);
    if (bitmap == NULL) {
        std::cout << "malloc failed";
    }

    printf("Searching prime numbers to: %ld", max);

    std::vector<pthread_t> threads(999);
    struct thread_data thread_data_array[999];
    //std::vector<thread_data> thread_stuff();
    start = clock();
    int threadcount;
    while ((testnum += 2) < max)
        if (!TEST(bitmap, testnum)) {
            thread_data_array[threadcount].testnum = testnum;
            thread_data_array[threadcount].max = max;
            pthread_create(&threads[threadcount],
                    &attr, set, (void *) &thread_data_array[threadcount]);
            ++threadcount;
        }
    theend = clock();
    std::cout << (((double) (theend - start)) / CLOCKS_PER_SEC) << std::endl;

    /* Wait for all threads to complete */
    for (int i = 0; i < threadcount; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("Main(): Waited on %d threads. Final value of count = %d. Done.\n",
            threadcount, count);

    /* This alg speeds things up by not checking even numbers,
     * 2 however is an exception so it must be added to the output. */
    printf("%d\t", 1);
    printf("%d\t", 2);
    count = 1;
    while ((count += 2) < max)
        if (!TEST(bitmap, count)) printf("%ld\t", count);

    /* Clean up and exit */
    free(bitmap);
    pthread_attr_destroy(&attr);
    pthread_exit(NULL);

}


