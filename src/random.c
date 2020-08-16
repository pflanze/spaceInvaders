#ifdef TEST_WITHOUT_IO

#include "random.h"
#include "CMWC.h"


void Random_Init(unsigned long seed) {
	CMWC_init(seed);
}

unsigned long Random(void) {
	return CMWC() & 255;
}

unsigned long Random32(void) {
	return CMWC();
}

#endif
