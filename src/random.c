#ifdef TEST_WITHOUT_IO

#include "random.h"
#include "PRNG.h"


EXPORTED
void random_init(unsigned long seed) {
	PRNG_init(seed);
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
	uint32_t dropbits= clz(ceil - 1);
	while (1) {
		uint32_t v = PRNG();
		v = v >> dropbits;
		if (v < ceil) {
			return v;
		}
	}
}


#endif
