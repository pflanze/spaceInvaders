#include "perhaps_assert.h"
#include <stdio.h>
#include <stdlib.h>


void checked_error(const char *msg) {
	fprintf(stderr, "%s\n", msg);
	abort();
}
