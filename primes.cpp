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
#include <math.h>

#include <limits.h>    /* for CHAR_BIT */
#include <stdint.h>   /* for uint32_t */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//typedef uint32_t word_t;
//enum {
//    BITS_PER_WORD = sizeof (word_t) * CHAR_BIT // need to revise calc
//};

enum {
    BITS_PER_WORD = 16
};
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)

/* return true if bit = 0 */
#define TEST(f,x)       (f[WORD_OFFSET(x)] & (1 << (BIT_OFFSET(x)/2)))
/* set bit at x to 0 */
#define SET(f,x)       (f[WORD_OFFSET(x)] |= (1 << (BIT_OFFSET(x)/2)))

/* Global variables since they are shared amoung threads */
unsigned char *bitmap = NULL;
pthread_mutex_t mutex;
pthread_cond_t prime_found;
unsigned long last_prime;
unsigned long max;

void test_print() {
    unsigned long j = 1;
    std::cout << "\n";
    while ((j += 2) < max)
        (!TEST(bitmap, j)) ? std::cout << "1" : std::cout << "0";
    std::cout << "\n";
}

void *set(void *threadarg) {
    unsigned long mom;
    unsigned long testnum;

    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&prime_found, &mutex);
    testnum = last_prime;
    std::cout << testnum;
    for (mom = 3 * testnum; mom < max; mom += testnum << 1) {
        SET(bitmap, mom);
    }
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    time_t start;
    time_t theend;
    pthread_attr_t attr;
    unsigned long j;
    unsigned long upperlim;
    unsigned long testnum = 1;
    int num_threads;
    int rc;
    int hits = 1;

    (argc > 1) ? num_threads = atol(argv[1]) : num_threads = 3;
    // at 2^32 or set to INT_MAX
    (argc > 2) ? max = atol(argv[2]) : max = 4294967296;
    upperlim = sqrt(max);

    pthread_t threads[num_threads];
    //struct thread_data thread_data_array[num_threads];

    /* For portability, explicitly create threads in a joinable state */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&prime_found, NULL);

    bitmap = (unsigned char*) calloc(max, 4);
    if (bitmap == NULL) {
        std::cout << "malloc failed";
    }

    test_print();

    std::cout << "Searching prime numbers to: " << max << std::endl;

    start = clock();
    /* spawn all threads */
    for (int k; k < num_threads; ++num_threads) {
        pthread_create(&threads[k], &attr, set, (void *) threads[k]);
    }
    /* when loop hits a 1 (not yet flagged as not prime) its assumed
     *  prime and a thread is started which will mark all multiples as non prime. */
    while ((testnum += 2) <= upperlim) {
        if (!TEST(bitmap, testnum)) {
            last_prime = testnum;
            pthread_cond_broadcast(&prime_found);
            ++hits;
        }
    }

    /* Wait for all threads to complete */
    for (int i = 0; i < num_threads; ++i) {
        rc = pthread_join(threads[i], NULL);
        if (rc) {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
    }
    theend = clock();

    std::cout << "Main(): Waited on " << hits << " threads." << std::endl;
    std::cout << "found " << hits << "in ";
    std::cout << (((double) (theend - start)) / CLOCKS_PER_SEC);
    std::cout << " seconds" << std::endl;

    test_print();

    /* This alg speeds things up by not checking even numbers,
     * 2 however is an exception so it must be added to the output. */
    printf("%d\t", 1);
    printf("%d\t", 2);
    j = 1;
    while ((j += 2) < max)
        if (!TEST(bitmap, j)) printf("%ld\t", j);

    std::cout << std::endl;
    /* Clean up and exit */
    free(bitmap);
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&prime_found);
    pthread_exit(NULL);

}


