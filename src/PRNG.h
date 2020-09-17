#ifndef _PRNG_H
#define _PRNG_H

#include <stdint.h>

void PRNG_init(uint32_t seed);

uint32_t PRNG();

#endif
