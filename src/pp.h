#ifndef _PP_H
#define _PP_H

#include "utils.h" /* bool */
#include "object.h"

#define PP(p)									\
	pp_helper((p), (p) ? (p)->vtable->pp : NULL, stderr, true)

#define PP_TO(p, out)							\
	pp_helper((p), (p) ? (p)->vtable->pp : NULL, out, true)

// same as PP_TO but does not print a newline (thus usable as part of
// implementing a pp method):
#define PP_TO_(p, out)							\
	pp_helper((p), (p) ? (p)->vtable->pp : NULL, out, false)


EXPORTED void pp_helper(const void *p,
						void(*_pp)(const struct ObjectVTable *const*this,
								   FILE *out),
						FILE *out,
						bool newline);


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
