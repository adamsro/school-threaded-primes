/*
 * Original Author: Robert M Adams (adamsro)
 * File: uniqify.cpp
<<<<<<< HEAD
 * Created: 2012 Feb 12, 18:35 by adamsro
=======
 * Created: 2012 Feb 15, 18:35 by adamsro
>>>>>>> timing
 * Last Modified: 2012 Feb 26, 20:00 by adamsro
 *
 *  Program will calculate primes up to 2^32 using Sieve of Erastothenes with
 * n threads
 *
 * helpful: http://www.fpx.de/fp/Software/
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define BITS_PER_WORD 16
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)
/* return true if bit = 0 */
#define TEST(f,x)       (f[WORD_OFFSET(x)] & (1 << (BIT_OFFSET(x)/2)))
/* set bit at x to 0 */
#define SET(f,x)       (f[WORD_OFFSET(x)] |= (1 << (BIT_OFFSET(x)/2)))

unsigned char *bitmap = NULL;
unsigned long max;
unsigned long hits;
int num_threads;
int finished_threads; /* indicates that all threads are complete */
unsigned long testnum;

/*
 * loops through odd numbers checking if bit has not be marked 'not prime'
 *  if prime is found, mark all multiples not prime.
 *
 * @param void* t integer,id
 */
void *mark_not_prime(void *t) {
    unsigned long mom;
    //long my_id = (long) t;
    /* search untill sqrt(n) */
    while (testnum * testnum <= max) {
        testnum += 2;
        if (!TEST(bitmap, testnum)) {
            for (mom = 3 * testnum; mom < max; mom += testnum << 1) {
                SET(bitmap, mom);
            }
        }
    }
    ++finished_threads; /* indicate thread is complete */
    pthread_exit(NULL);
}

/*
 * Print all prime numbers found.
 * Must be called after threads all signal complete.
 */
void print_primes() {
    /* This alg speeds things up by not checking even numbers,
     * 2 however is an exception so it must be added to the output. */
    printf("%d\t", 1);
    printf("%d\t", 2);
    unsigned long k = 1;
    while ((k += 2) < max)
        if (!TEST(bitmap, k)) std::cout << k << "\t";
}

int main(int argc, char *argv[]) {
    pthread_attr_t attr;
    time_t start;
    time_t theend;
    long ncpus;
    double total_time;

    (argc > 1) ? num_threads = atol(argv[1]) : num_threads = 8;
    // at 2^32 or set to INT_MAX
    (argc > 2) ? max = atol(argv[2]) : max = 4294967296;
    //upperlim = sqrt(max);

    // code for test looping
    for (unsigned long mx = 1024; mx <= 4294967296; mx = mx * 2) {
        for (unsigned long food = 1; food <= 10; ++food) {
            max = mx;
            num_threads = food;

            pthread_t threads[num_threads];
            long thread_ids[num_threads];
            for (int i = 0; i < num_threads; ++i) {
                thread_ids[i] = i + 1;
            }
            bitmap = (unsigned char*) calloc(max, 4);
            if (bitmap == NULL) {
                std::cout << "malloc failed";
            }
            /* For portability, explicitly create threads in a joinable state */
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

            //std::cout << "Searching for primes 0 to " << max << "\n";

            start = clock();
            testnum = 1;
            hits = 1;
            for (int i = 0; i < num_threads; ++i) {
                pthread_create(&threads[i], &attr, mark_not_prime, (void *) thread_ids[i]);
            }
            while (finished_threads < num_threads) {
                ; /* until all threads are complete */
            }
            for (int i = 0; i < num_threads; i++) {
                pthread_join(threads[i], NULL);
            }
            theend = clock();

            ncpus = sysconf(_SC_NPROCESSORS_ONLN);
            total_time = (((double) (theend - start)) / (double) CLOCKS_PER_SEC);

            printf("%ld\t",max);
            printf("%d\n" ,num_threads);
            printf("%.6f\t", total_time / (num_threads < ncpus ? num_threads : ncpus));

            //print_primes();

            /* Clean up and exit */
            free(bitmap);
            pthread_attr_destroy(&attr);
        }
    }
    pthread_exit(NULL);
}


