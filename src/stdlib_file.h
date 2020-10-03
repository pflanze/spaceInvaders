#ifndef _STDLIB_FILE_H
#define _STDLIB_FILE_H

#include <stdio.h>
#include "object.h"

EXPORTED void die_errno(const char *msg, const char *arg);



#define OUTFILE_PATHSIZ 100

#define OUTFILE_INTERFACE						\
	const char *(*path)(const void *this);

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

EXPORTED void OutFile_xclose_and_free(void *this);
// ^ `this` must satisfy `struct OutFileInterface *`

#endif /* _STDLIB_FILE_H */
