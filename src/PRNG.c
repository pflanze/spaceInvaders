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


/* used to record the fact that PRNG_mixin was called even if randomvalue was 0 */
static
uint64_t mixin_count = 0;

EXPORTED
void PRNG_init(uint32_t seed) {
	x = seed;
	w = 0;
	mixin_count = 0;
	// these may not be needed:
	PRNG();
	PRNG();
}

EXPORTED
void PRNG_mixin(uint32_t randomvalue) {
	mixin_count++;
	x ^= randomvalue ^ mixin_count;
}
