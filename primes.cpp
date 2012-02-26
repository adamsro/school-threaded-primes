
/*
 * Original Author: Robert M Adams (adamsro)
 * File: uniqify.cpp
 * Created: 2012 Feb 1, 18:35 by adamsro
 * Last Modified: 2012 Feb 7, 20:00 by adamsro
 *
 * File contains a filter which spawns n sort processses based on a command
 * line argument. The primary purpose of this is project to learn pipes.
 *
 * helpful: http://tldp.org/LDP/lpg/node11.html
 * and: http://stackoverflow.com/questions/1381089/multiple-fork-concurrency
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define BITS_PER_WORD 16
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)
/* return true if bit = 0 */
#define TEST(f,x)       (f[WORD_OFFSET(x)] & (1 << (BIT_OFFSET(x)/2)))
/* set bit at x to 0 */
#define SET(f,x)       (f[WORD_OFFSET(x)] |= (1 << (BIT_OFFSET(x)/2)))

unsigned char *bitmap = NULL;
unsigned long max;
int num_threads;
int hits = 1;
int finished_threads; /* indicates that all threads are complete */

void *mark_not_prime(void *t) {
    long my_id = (long) t;
    unsigned long mom;
    unsigned long testnum;
    printf("Starting mark_not_prime(): thread %ld\n", my_id);
    while ((testnum += 2) <= max) {
        if (!TEST(bitmap, testnum)) {
            ++hits;
            for (mom = 3 * testnum; mom < max; mom += testnum << 1) {
                SET(bitmap, mom);
            }
        }
    }
    ++finished_threads; /* indicate thread is complete */
    printf("Ending mark_not_prime(): thread %ld\n", my_id);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_attr_t attr;

    (argc > 1) ? num_threads = atol(argv[1]) : num_threads = 3;
    // at 2^32 or set to INT_MAX
    (argc > 2) ? max = atol(argv[2]) : max = 4294967296;
    //upperlim = sqrt(max);

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
    for (int i = 0; i < num_threads; ++i) {
        pthread_create(&threads[i], &attr, mark_not_prime, (void *) thread_ids[i]);
    }

    while (finished_threads < num_threads) {
        ; /* until all threads are complete */
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("Main(): Waited on %d threads. Done.\n", num_threads);

    /* This alg speeds things up by not checking even numbers,
     * 2 however is an exception so it must be added to the output. */
    printf("%d\t", 1);
    printf("%d\t", 2);
    unsigned long k = 1;
    while ((k += 2) < max)
        if (!TEST(bitmap, k)) std::cout << k << "\t";

    /* Clean up and exit */
    pthread_attr_destroy(&attr);
    pthread_exit(NULL);

}


