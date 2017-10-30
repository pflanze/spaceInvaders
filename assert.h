//assert.h

#ifndef _ASSERT_H
#define _ASSERT_H

#ifdef TEST_WITHOUT_IO
#		include <assert.h>
#else
#		define assert(expr)
#endif

#endif

