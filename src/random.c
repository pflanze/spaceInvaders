#ifdef TEST_WITHOUT_IO

#include <stdio.h>
#include "random.h"
#include "PRNG.h"


EXPORTED
void Random_Init(unsigned long seed) {
	PRNG_init(seed);
}

#ifdef UNUSED
EXPORTED
unsigned long Random(void) {
	return PRNG() & 255;
}
#endif

EXPORTED
unsigned long Random32(void) {
	return PRNG();
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
	uint32_t v;
	uint32_t dropbits= clz(ceil - 1);
	while (1) {
		v = PRNG();
		printf("random_uint32: v=%u, ceil=%u\n", v, ceil);
		v = v >> dropbits;
		printf("random_uint32: => v=%u\n", v);
		if (v < ceil) {
			return v;
		}
		printf("random_uint32: try again (v=%u, ceil=%u)\n", v, ceil);
	}
}


#endif
