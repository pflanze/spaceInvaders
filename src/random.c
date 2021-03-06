#ifdef TEST_WITHOUT_IO

#include "random.h"
#include "PRNG.h"
#include "perhaps_assert.h"


EXPORTED
void random_init(uint32_t seed) {
	PRNG_init(seed);
}

EXPORTED
void random_mixin(uint32_t randomvalue) {
	PRNG_mixin(randomvalue);
}


/* static */
/* uint32_t bitlength(uint32_t x) { */
/* 	return 32 - __builtin_clz(x); */
/* } */

static
uint32_t clz(uint32_t x) {
	return __builtin_clz(x);
}

EXPORTED
uint32_t random_uint32(uint32_t ceil) {
	assert(ceil >= 1);
	if (ceil == 1) {
		return 0;
	} else {
		uint32_t dropbits= clz(ceil - 1);
		while (1) {
			uint32_t v = PRNG() >> dropbits;
			if (v < ceil) {
				return v;
			}
		}
	}
}


#endif
