#ifdef TEST_WITHOUT_IO

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
		v = v >> clz(ceil);
		if (v < ceil) {
			return v;
		}
	}
}


#endif
