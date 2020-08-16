#ifndef _CMWC_H
#define _CMWC_H

extern unsigned long CMWC_Q[4096], CMWC_c, CMWC_i;

void CMWC_init(unsigned long seed);

unsigned long CMWC(void);

#endif
