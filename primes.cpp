
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define NUM_THREADS  3

#define BITS_PER_WORD 16
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)
/* return true if bit = 0 */
#define TEST(f,x)       (f[WORD_OFFSET(x)] & (1 << (BIT_OFFSET(x)/2)))
/* set bit at x to 0 */
#define SET(f,x)       (f[WORD_OFFSET(x)] |= (1 << (BIT_OFFSET(x)/2)))

pthread_mutex_t count_mutex;
pthread_cond_t count_threshold_cv;
unsigned char *bitmap = NULL;
unsigned long last_prime;
unsigned long max;
int num_threads;
int hits = 1;

/*
    Check the value of count and signal waiting thread when condition is
    reached.  Note that this occurs while mutex is locked.
     */
void *get_primes(void *t) {
    long my_id = (long) t;
    unsigned long testnum = 1;
    unsigned long upperlim;
    printf("Starting get_primes(): thread %ld\n", my_id);
    upperlim = sqrt(max);
    while ((testnum += 2) <= upperlim) {
        if (!TEST(bitmap, testnum)) {
            pthread_mutex_lock(&count_mutex);
            last_prime = testnum;
            pthread_cond_signal(&count_threshold_cv);
            printf("Just sent signal.\n");
            pthread_mutex_unlock(&count_mutex);
            ++hits;
        }
        /* Do some work so threads can alternate on mutex lock */
        sleep(1);
    }
    printf("Ending get_primes(): thread %ld\n", my_id);



    pthread_exit(NULL);
}

void *mark_not_prime(void *t) {
    long my_id = (long) t;
    unsigned long mom;
    unsigned long testnum;

    printf("Starting mark_not_prime(): thread %ld\n", my_id);
    pthread_mutex_lock(&count_mutex);
    printf("mark_not_prime(): thread %ld going into wait...\n", my_id);
    pthread_cond_wait(&count_threshold_cv, &count_mutex);
    printf("mark_not_prime(): thread %ld Condition signal received.\n", my_id);
    testnum = last_prime;
    std::cout << "got prime: " << testnum << std::endl;
    for (mom = 3 * testnum; mom < max; mom += testnum << 1) {
        SET(bitmap, mom);
    }
    printf("Ending mark_not_prime(): thread %ld\n", my_id);
    pthread_mutex_unlock(&count_mutex);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int i;
    long t1 = 1, t2 = 2, t3 = 3;
    pthread_t threads[3];
    pthread_attr_t attr;

    (argc > 1) ? num_threads = atol(argv[1]) : num_threads = 3;
    // at 2^32 or set to INT_MAX
    (argc > 2) ? max = atol(argv[2]) : max = 4294967296;

    bitmap = (unsigned char*) calloc(max, 4);
    if (bitmap == NULL) {
        std::cout << "malloc failed";
    }

    /* Initialize mutex and condition variable objects */
    pthread_mutex_init(&count_mutex, NULL);
    pthread_cond_init(&count_threshold_cv, NULL);

    /* For portability, explicitly create threads in a joinable state */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&threads[0], &attr, mark_not_prime, (void *) t1);
    pthread_create(&threads[0], &attr, mark_not_prime, (void *) t2);
    pthread_create(&threads[1], &attr, get_primes, (void *) t3);

    /* Wait for all threads to complete */
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("Main(): Waited on %d threads. Done.\n",
            NUM_THREADS);

    /* Clean up and exit */
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&count_mutex);
    pthread_cond_destroy(&count_threshold_cv);
    pthread_exit(NULL);

}


