#ifndef _STDLIB_UTILS_H
#define _STDLIB_UTILS_H

#include <stdlib.h>

#define NORETURN __attribute__ ((noreturn)) /* works on gcc and clang */

void xmalloc_abort(size_t size) NORETURN;

static inline
void *xmalloc(size_t size) {
	void *p= malloc(size);
	if (!p) {
		xmalloc_abort(size);
	}
	return p;
}

#define LET_XMALLOC(var, type) type *var = xmalloc(sizeof(*var))


#endif // _STDLIB_UTILS_H
