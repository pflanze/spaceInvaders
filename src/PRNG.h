#ifndef _PRNG_H
#define _PRNG_H

#include <stdint.h>

EXPORTED void PRNG_init(uint32_t seed);

EXPORTED uint32_t PRNG();

EXPORTED void PRNG_mixin(uint32_t randomvalue);

#endif
