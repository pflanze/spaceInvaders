#ifndef _PERHAPS_ASSERT_H
#define _PERHAPS_ASSERT_H

#ifdef TEST_WITHOUT_IO
#		include <assert.h>
#else
#		define assert(expr)
#endif

#endif

