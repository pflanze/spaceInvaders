/*
 http://www.gamedev.net/topic/512260-number-generator-cmwc-4096-and-implementation/
*/

#include <string.h>

unsigned long CMWC_Q[4096], CMWC_c=123, CMWC_i=4095;

void CMWC_init(unsigned long seed) {
	for (int i=0; i<4096; i++) {
		CMWC_Q[i]= seed;
	}
	CMWC_c=123;
	CMWC_i=4095;
}

unsigned long CMWC(void)
{
	unsigned long long t, a=18782LL;
	unsigned long x, m=0xFFFFFFFE;

	CMWC_i = (CMWC_i + 1) & 4095;
	t = a * CMWC_Q[CMWC_i] + CMWC_c;
	CMWC_c = (t >> 32);
	x = t + CMWC_c;
	if(x < CMWC_c)
	{
		++x;
		++CMWC_c;
	}
	return (CMWC_Q[CMWC_i] = m - x);
}


