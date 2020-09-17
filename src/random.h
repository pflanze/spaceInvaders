#ifndef _RANDOM_H
#define _RANDOM_H

#include <stdint.h> // C11

/*Random number generator;
  Linear congruential generator 
  from Numerical Recipes by Press et al.
  Jonathan Valvano

  How to use: 
  Call random_init once with a seed. For example
      random_init(1);
      random_init(NVIC_CURRENT_R);
  Call Random over and over to get a new random number. For example
      m = Random32()%60; // returns a random number from 0 to 59
      p = Random();      // returns a random number 0 to 255
*/
EXPORTED void random_init(unsigned long seed);

// A number [0..ceil)
EXPORTED uint32_t random_uint32(uint32_t ceil);


#endif
