#include "pp.h"

#include <stdio.h>

const char* bool_show(bool v) {
    return v ? "true" : "false";
}

void flush() {
    fflush(stdout);
}

void pp_helper(void* p, void(*_pp)(void* this)) {
    _pp(p);
    printf("\n");
    flush();
}
