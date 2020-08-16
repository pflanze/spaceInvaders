#include "pp.h"

#include <stdio.h>

const char* bool_show(bool v) {
    return v ? "true" : "false";
}

void pp_helper(void* p,
			   void(*_pp)(void* this, FILE* out),
			   FILE* out) {
    _pp(p, out);
    PP_PRINTF("\n");
}
