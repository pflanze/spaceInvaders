//Buttons.h

#ifndef _BUTTONS_H
#define _BUTTONS_H

#include "utils.h"

#ifdef TEST_WITHOUT_IO
extern bool GPIO_PORTE_DATA_R;
#endif

EXPORTED bool Pressfire_B1(void);
EXPORTED bool Pressfire_B2(void);


#endif
