#ifdef TEST_WITHOUT_IO

#include <stdio.h>
#include "random.h"
#include "CMWC.h"


EXPORTED
void Random_Init(unsigned long seed) {
	CMWC_init(seed);
}

#ifdef UNUSED
EXPORTED
unsigned long Random(void) {
	return CMWC() & 255;
}
#endif

EXPORTED
unsigned long Random32(void) {
	return CMWC();
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
	while (1) {
		v = CMWC();
		printf("random_uint32: v=%u, ceil=%u\n", v, ceil);
		v = v >> clz(ceil);
		printf("random_uint32: => v=%u\n", v);
		if (v < ceil) {
			return v;
		}
		printf("random_uint32: try again (v=%u, ceil=%u)\n", v, ceil);
	}
}


#endif
