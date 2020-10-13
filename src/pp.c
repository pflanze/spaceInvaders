#include "pp.h"

#include <stdio.h>

EXPORTED
const char *bool_show(bool v) {
    return v ? "true" : "false";
}

EXPORTED
void pp_helper(const struct ObjectVTable *const*_this,
			   FILE *out,
			   bool newline) {
	const struct ObjectInterface *this = (const struct ObjectInterface *)_this;
	if (this) {
		VCALL(pp, this, out);
	} else {
		PP_PRINTF("NULL");
	}
	if (newline) {
		PP_PRINTF("\n");
	}
}
