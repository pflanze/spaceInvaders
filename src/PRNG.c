/*
https://en.wikipedia.org/wiki/Middle_square_method#Middle_Square_Weyl_Sequence_PRNG

TODO: *is* this the full code for the PRNG?
*/

#include "PRNG.h"

static
uint64_t x = 0, w = 0;

static
const uint64_t s = 0xb5ad4eceda1ce2a9;

EXPORTED
uint32_t PRNG() {
    x *= x; 
    x += (w += s); 
    return x = (x>>32) | (x<<32);
}

EXPORTED
void PRNG_init(uint32_t seed) {
	x = seed;
	w = 0;
	PRNG();
	PRNG();
}

EXPORTED
void PRNG_mixin(uint32_t randomvalue) {
	x ^= randomvalue;
}
