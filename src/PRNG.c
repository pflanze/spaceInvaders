/*
 http://www.gamedev.net/topic/512260-number-generator-cmwc-4096-and-implementation/
*/

#include <string.h>
#include <stdio.h>


unsigned long CMWC_Q[4096], CMWC_c=123, CMWC_i=4095;

unsigned long PRNG(void)
{
	unsigned long long t, a=18782LL;
	unsigned long x, m=0xFFFFFFFE;

	CMWC_i = (CMWC_i + 1) & 4095;
	printf("CMWC_c=%lu, CMWC_i=%lu\n", CMWC_c, CMWC_i);
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

void PRNG_init(unsigned long seed) {
	for (unsigned long i=0; i<4096; i++) {
		CMWC_Q[i]= seed + i;
	}
	CMWC_c=123;
	CMWC_i=4095;

	// try to properly mix up the state before use (yes, I *really* have no
	// idea of cryptography):
	unsigned long _i = 0;
	for (unsigned long i=0; i<4096; i++) {
		PRNG();
		CMWC_Q[i] ^= (seed - i);
		_i = (_i + i + CMWC_Q[_i]) & 4095;
		printf("i=%lu, _i=%lu, CMWC_Q[i]=%lu\n", i, _i, CMWC_Q[i]);
		CMWC_Q[_i] ^= CMWC_Q[i];
	}
}

