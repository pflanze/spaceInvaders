#include "stdlib_utils.h"
#include <stdio.h>

void xmalloc_abort(size_t size) {
	fprintf(stderr, "out of memory attempting to allocate %ld bytes\n", size);
	abort();
}

