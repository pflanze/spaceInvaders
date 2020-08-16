#include "pp.h"

#include <stdio.h>

EXPORTED
const char* bool_show(bool v) {
    return v ? "true" : "false";
}

EXPORTED
void pp_helper(void* p,
			   void(*_pp)(void* this, FILE* out),
			   FILE* out) {
    _pp(p, out);
    PP_PRINTF("\n");
}
