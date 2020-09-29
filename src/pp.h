#ifndef _PP_H
#define _PP_H

#include "utils.h" /* bool */
#include "object.h"

#define PP(p)									\
	pp_helper((p), (p)->vtable->pp, stdout)

#define PP_TO(p, out)							\
	pp_helper((p), (p)->vtable->pp, out)

EXPORTED void pp_helper(const void *p,
						void(*_pp)(const void *this, FILE *out),
						FILE *out);


// Helper functions

EXPORTED const char *bool_show(bool v);

// Ugly? Unhygienic, but at least name-prefixed:

#ifdef FLUSH
#  define PP_FLUSH fflush(out)
#  define PP_PRINTF(fmt, args...) { fprintf(out, fmt, ##args); PP_FLUSH; }
#else
#  define PP_FLUSH
#  define PP_PRINTF(fmt, args...) { fprintf(out, fmt, ##args); }
#endif

#endif // _PP_H
