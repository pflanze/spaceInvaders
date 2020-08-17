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

#endif
