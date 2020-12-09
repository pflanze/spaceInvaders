#ifndef _STDLIB_FILE_H
#define _STDLIB_FILE_H

#include <stdio.h>
#include "object.h"
#include "stdlib_utils.h"


EXPORTED void die_errno_1(const char *msg, const char *arg) NORETURN;
EXPORTED void die_errno_2(const char *msg, const char *arg1, const char *arg2)
	NORETURN;

// dies if cmd couldn't be run
EXPORTED int xsystem(const char *cmd);

// dies if cmd doesn't exit with code 0
EXPORTED void xxsystem(const char *cmd);


#define OUTFILE_PATHSIZ 100

#define OUTFILE_INTERFACE						\
	const char *(*path)(const struct OutFileVTable *const*this);

struct OutFileVTable {
	OBJECT_INTERFACE;
	OUTFILE_INTERFACE;
};


// base class fields for OutFile classes
#define OUTFILE_BASE							\
	const struct OutFileVTable *vtable;			\
	FILE *out

struct OutFileInterface {
	OUTFILE_BASE;
};

struct SimpleOutFile {
	OUTFILE_BASE;
	const char *path;
};

struct NumberedOutFile {
	OUTFILE_BASE;
	char path[OUTFILE_PATHSIZ];
};


EXPORTED struct SimpleOutFile *
SimpleOutFile_xopen(const char *path);

EXPORTED struct NumberedOutFile *
NumberedOutFile_xopen(unsigned int i, const char *name);

EXPORTED void _OutFile_xclose_and_free(const struct OutFileVTable *const*this);
#define OutFile_xclose_and_free(this)			\
	_OutFile_xclose_and_free(&(this)->vtable)

#endif /* _STDLIB_FILE_H */
