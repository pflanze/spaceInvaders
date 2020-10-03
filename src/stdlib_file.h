#ifndef _STDLIB_FILE_H
#define _STDLIB_FILE_H

#include <stdio.h>
#include "object.h"

EXPORTED void die_errno(const char *msg, const char *arg);



#define OUTFILE_PATHSIZ 100

#ifdef DIRECT
#  define OUTFILE_INTERFACE						\
	const char *(*path)(const void *this);
#else
#  define OUTFILE_INTERFACE						\
	const char *(*path)(const void *this);		\
	FILE *(*out)(const void *this);
#endif

struct OutFileVTable {
	OBJECT_INTERFACE;
	OUTFILE_INTERFACE;
};

// abstract base class, don't instantiate
struct OutFileInterface {
	const struct OutFileVTable *vtable;
#ifdef DIRECT
	FILE *out;
#endif
};

struct SimpleOutFile {
#ifdef DIRECT
	struct OutFileInterface base;
#else
	const struct OutFileVTable *vtable;
	FILE *out;
#endif
	const char *path;
};

struct NumberedOutFile {
#ifdef DIRECT
	struct OutFileInterface base;
#else
	const struct OutFileVTable *vtable;
	FILE *out;
#endif
	char path[OUTFILE_PATHSIZ];
};


EXPORTED struct SimpleOutFile *
SimpleOutFile_xopen(const char *path);

EXPORTED struct NumberedOutFile *
NumberedOutFile_xopen(unsigned int i, const char *name);

EXPORTED void OutFile_xclose_and_free(struct OutFileInterface *this);


#endif /* _STDLIB_FILE_H */
