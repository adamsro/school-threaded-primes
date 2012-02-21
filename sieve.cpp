#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <limits.h>    /* for CHAR_BIT */
#include <stdint.h>   /* for uint32_t */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
#define TEST(f,x)       (*(f+WORD_OFFSET(x)) & (1 << (BIT_OFFSET(x)/2)))
/* set bit at x */
#define SET(f,x)       (*(f+WORD_OFFSET(x)) |= (1 << (BIT_OFFSET(x)/2)))

int main(int argc, char *argv[]) {
    unsigned char *bitmap = NULL;
    unsigned long testnum = 1, max, mom, hits = 1, count;
    time_t start;
    time_t theend;

    (argc > 1) ? max = atol(argv[1]) : max = INT_MAX;

    bitmap = (unsigned char*) malloc(max >> 4);
    if (bitmap == NULL) {
        std::cout << "malloc failed";
        //throw (Exception("calloc failed", __LINE__, errno));
    }

    printf("Searching prime numbers to: %ld", max);

    start = clock();
    while ((testnum += 2) < max)
        if (!TEST(bitmap, testnum)) {
            if (++hits % 2000L == 0) {
                printf(" %ld. prime number\x0d", hits);
                fflush(stdout);
            }
            for (mom = 3 * testnum; mom < max; mom += testnum << 1)
                SET(bitmap, mom);
        }
    theend = clock();
    std::cout << (((double) (theend - start)) / CLOCKS_PER_SEC) << std::endl;

    /* This alg speeds things up by not checking even numbers,
     * 2 however is an exception so it must be added to the output. */
    printf("%d\t", 1);
    printf("%d\t", 2);
    count = 1;
    while ((count += 2) < max)
        if (!TEST(bitmap, count)) printf("%ld\t", count);

    free(bitmap);
    return 0;
}