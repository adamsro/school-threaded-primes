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

/* for threads */

struct thread_data {
    pthread_t thread_id;
    unsigned long max;
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

/* is bit == 1? */
#define TEST(f,x)       *(f+WORD_OFFSET(x)) & (1 << (BIT_OFFSET(x)/2))
/* set bit at x to 1*/
#define SET(f,x)       *(f+WORD_OFFSET(x)) |= (1 << (BIT_OFFSET(x)/2))
/* invert bit */
#define CLEAR(f,x)       *(f+WORD_OFFSET(n)) &= ~(1 << (BIT_OFFSET(n)/2))

void *set(void *threadarg) {
    unsigned long mom;
    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;
    printf("my_data->testnum: %ld\n", my_data->testnum);
    for (mom = 3 * my_data->testnum; mom < my_data->max; mom += my_data->testnum << 1) {
        //©SET(bitmap, mom);
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

    time_t start;
    time_t theend;
    pthread_attr_t attr;
    unsigned long testnum = 1;
    unsigned long max;
    unsigned long j;
    int rc;
    int hits = 1;
    int num_threads;
    unsigned long upperlim;
    unsigned char *bitmap = NULL;

    (argc > 1) ? num_threads = atol(argv[1]) : num_threads = 3;

    upperlim = sqrt(max);
    // at 2^32 or set to INT_MAX
    (argc > 2) ? max = atol(argv[2]) : max = 4294967296;

    //pthread_t threads[num_threads];
    struct thread_data thread_data_array[num_threads];

    /* For portability, explicitly create threads in a joinable state */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    //bitmap = (unsigned char*) malloc(max >> 4);
    bitmap = (unsigned char*) calloc(max, 4);
    if (bitmap == NULL) {
        std::cout << "malloc failed";
    }

    std::cout << "Searching prime numbers to: " << max << std::endl;
    //printf("Searching prime numbers to: %ld", max);
    j = 1;
    while ((j += 2) < 100)
        (!TEST(bitmap, j)) ? std::cout << "1" : std::cout << "0";
    std::cout << "\n";
    SET(bitmap, 3);
    j = 1;
    while ((j += 2) < 100)
        (!TEST(bitmap, j)) ? std::cout << "1" : std::cout << "0";
    std::cout << "\n";

    start = clock();
    for (int k; k < num_threads; ++num_threads) {
        pthread_create(&thread_data_array[k].thread_id,
                &attr, set, (void *) &thread_data_array[k]);
    }
    unsigned long mom;
    while ((testnum += 2) <= upperlim) {
        if (!TEST(bitmap, testnum)) {
            thread_data_array[hits].testnum = testnum;
            thread_data_array[hits].max = max;
            //thread_data_array[hits].bitmap = bitmap;
            for (mom = 3 * testnum; mom < max; mom += testnum << 1)
                SET(bitmap, mom);
            ++hits;
        }
    }


    /* Wait for all threads to complete */
    for (int i = 0; i < hits; ++i) {
        rc = pthread_join(thread_data_array[i].thread_id, NULL);
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
    pthread_exit(NULL);

}


