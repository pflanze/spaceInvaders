#ifdef TEST_WITHOUT_IO

#include <stdlib.h>


void Random_Init(unsigned long seed) {
	srand(seed);
}

unsigned long Random(void) {
	return rand();
}

unsigned long Random32(void) {
	return rand();
}

#endif
