#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <limits.h>    /* for CHAR_BIT */
#include <stdint.h>   /* for uint32_t */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef uint32_t word_t;

//enum {
//    BITS_PER_WORD = sizeof (word_t) * CHAR_BIT
//};

enum {
    BITS_PER_WORD = 16 // assuming CHAR_BIT == 4 ?
};

#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)

/* return true if bit = 0 */
#define TEST(f,x)       (f[WORD_OFFSET(x)] & (1 << (BIT_OFFSET(x)/2)))
/* set bit at x to 0 */
#define SET(f,x)       (f[WORD_OFFSET(x)] |= (1 << (BIT_OFFSET(x)/2)))

unsigned char *bitmap = NULL;

void test_print(unsigned long max) {
    unsigned long j = 1;
    std::cout << "\n";
    while ((j += 2) < max)
        (!TEST(bitmap, j)) ? std::cout << "1" : std::cout << "0";
    std::cout << "\n";
}

int main(int argc, char *argv[]) {

    unsigned long testnum = 1, max, mom, hits = 1, count;
    unsigned long upperlim;
    time_t start;
    time_t theend;

    std::cout << "char_bit is: " << CHAR_BIT << std::endl;
    std::cout << "BITS_PER_WORD is: " << BITS_PER_WORD << std::endl;

    // at 2^32 or set to INT_MAX
    (argc > 1) ? max = atol(argv[1]) : max = 4294967296;
    upperlim = sqrt(max);

    bitmap = (unsigned char*) calloc(max, 4);
    if (bitmap == NULL) {
        std::cout << "calloc failed";
        exit(EXIT_FAILURE);
    }
    test_print(max);
    printf("Searching prime numbers to: %ld \n", max);

    start = clock();
    while ((testnum += 2) <= upperlim) {
        if (!TEST(bitmap, testnum)) {
            for (mom = 3 * testnum; mom < max; mom += testnum << 1) {
                std::cout << testnum << " " << mom << std::endl;
                SET(bitmap, mom);
            }
        }
    }
    theend = clock();

    std::cout << "found " << hits << " in ";
    std::cout << (((double) (theend - start)) / CLOCKS_PER_SEC);
    std::cout << " seconds" << std::endl;

    /* This alg speeds things up by not checking even numbers,
     * 2 however is an exception so it must be added to the output. */
    printf("%d\t", 1);
    printf("%d\t", 2);
    count = 1;
    while ((count += 2) < max)
        if (!TEST(bitmap, count)) std::cout << count << "\t";

    test_print(max);
    std::cout << std::endl;

    free(bitmap);
    return 0;
}

