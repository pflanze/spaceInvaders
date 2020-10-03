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



#ifdef DIRECT
// abstract base class, don't instantiate
struct OutFile {
	FILE *out;
};
#endif


// bad name in case of DIRECT, should it be `SubclassOfOutFileInterface`?
struct OutFileInterface {
	const struct OutFileVTable *vtable;
#ifdef DIRECT
	struct OutFile super;
#endif
};

struct SimpleOutFile {
	const struct OutFileVTable *vtable;
#ifdef DIRECT
	struct OutFile super;
#else
	FILE *out;
#endif
	const char *path;
};

struct NumberedOutFile {
	const struct OutFileVTable *vtable;
#ifdef DIRECT
	struct OutFile super;
#else
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
