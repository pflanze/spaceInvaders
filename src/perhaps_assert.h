#ifndef _PERHAPS_ASSERT_H
#define _PERHAPS_ASSERT_H

#ifdef TEST_WITHOUT_IO
#		include <assert.h>
#else
#		define assert(expr)
#endif

// NOTE: do not define NDEBUG (man 3 assert), or XSNPRINTF will be unchecked!
#define XSNPRINTF(str, n, ...) \
	assert(snprintf(str, n, __VA_ARGS__) < n)

#endif

