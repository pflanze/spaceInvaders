#ifndef _PP_H
#define _PP_H

#include "utils.h" /* bool */
#include "object.h"

#define PP(p)					\
	pp_helper((p), (p)->vtable->pp)

void pp_helper(void* p,
			   void(*_pp)(void* this, FILE* out));


// Helper functions

const char* bool_show(bool v);

// Ugly? Unhygienic, but at least name-prefixed:

#ifdef FLUSH
#  define PP_FLUSH fflush(out)
#  define PP_PRINTF(fmt, args...) { fprintf(out, fmt, ##args); PP_FLUSH; }
#else
#  define PP_FLUSH
#  define PP_PRINTF(fmt, args...) { fprintf(out, fmt, ##args); }
#endif

#endif // _PP_H
