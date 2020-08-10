#ifndef _PP_H
#define _PP_H

#include "utils.h" /* bool */
#include "object.h"

#define PP(p)					\
    pp_helper((p), (p)->vtable->pp)

void pp_helper(void* p, void(*_pp)(void* this));


// Helper functions

const char* bool_show(bool v);

// Ugly ones?

void flush();

#ifdef FLUSH
#  undef FLUSH
#  define FLUSH flush()
#else
#  define FLUSH
#endif

#endif // _PP_H
