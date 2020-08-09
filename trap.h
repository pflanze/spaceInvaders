#ifndef _TRAP_H
#define _TRAP_H

#include <sys/types.h>
#include <signal.h>

static inline
void trap() {
        raise(SIGTRAP);
        // alternative: asm("int $3");
}

#ifdef DEBUG
#define TRAP() trap()
#else
#define TRAP() 
#endif

#endif
