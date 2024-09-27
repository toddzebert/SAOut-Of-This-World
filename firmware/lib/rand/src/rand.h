#ifndef RAND_H_
#define RAND_H_

#include <stdio.h>

/*
 * rand.h - LFSR-based random number generation
 * Released to Public Domain 08-05-2024 E. Brombaugh
 */
 
/*
 * Common function
 * seed = 0 - advances LFSR state and returns result
 * seed = anything else - set seed to that value
 * bit = 1-32 - number of bits to generate
 */
uint32_t rnd_fun(uint32_t seed, uint8_t bits);

/* generates 31 bits PRN */
int rand(void);

/* set seed */
void srand(unsigned int seed);

#endif /* RAND_H_ */
