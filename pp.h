#ifndef _PP_H
#define _PP_H

#include "utils.h" /* bool */
#include "object.h"

#define PP(p)					\
    {						\
    V(pp, p);					\
    printf("\n");				\
    FLUSH;					\
    }

// Helper functions

const char* bool_show(bool v);

// Ugly ones?

void flush();
#define FLUSH flush()


#endif // _PP_H
