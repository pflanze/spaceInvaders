#ifndef _PRNG_H
#define _PRNG_H

extern unsigned long CMWC_Q[4096], CMWC_c, CMWC_i;

void PRNG_init(unsigned long seed);

unsigned long PRNG(void);

#endif
