#include "pp.h"

#include <stdio.h>

EXPORTED
const char *bool_show(bool v) {
    return v ? "true" : "false";
}

EXPORTED
void pp_helper(const void *p,
			   void(*_pp)(const struct ObjectVTable *const*this, FILE *out),
			   FILE *out,
			   bool newline) {
	if (p) {
		_pp(p, out);
	} else {
		PP_PRINTF("NULL");
	}
	if (newline) {
		PP_PRINTF("\n");
	}
}
