#ifndef _RANDOM_H
#define _RANDOM_H

#include <stdint.h> // C11

/*
  Pseudo random number generator.

  How to use:
  
  Call random_init once with a seed. For example
      random_init(1);
      random_init(NVIC_CURRENT_R);

  Then call random_uint32 with the number one above the maximum number you
  accept, as many times as you wish.
  
*/

EXPORTED void random_init(uint32_t seed);

EXPORTED void random_mixin(uint32_t randomvalue);


// A number [0..ceil)
EXPORTED uint32_t random_uint32(uint32_t ceil);


#endif
