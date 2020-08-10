#include "pp.h"

#include <stdio.h>

const char* bool_show(bool v) {
    return v ? "true" : "false";
}

void flush() {
    fflush(stdout);
}
