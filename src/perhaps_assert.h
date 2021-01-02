#ifndef _PERHAPS_ASSERT_H
#define _PERHAPS_ASSERT_H

#include "utils.h" /* bool */

#ifdef TEST_WITHOUT_IO
#		include <assert.h>
#else
#		define assert(expr)
#endif


// same as assert but always checked:

void checked_error(const char *msg);

static inline
void checked_check(bool result, const char *msg) {
	if (! result) {
		checked_error(msg);
	}
}

#define CHECKED(expr)							\
	checked_check(expr, #expr)


// checked operations:

#define XSNPRINTF(str, n, ...)					\
	CHECKED(snprintf(str, n, __VA_ARGS__) < n)

#endif

