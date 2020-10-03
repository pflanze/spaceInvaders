#ifndef _STDLIB_FILE_H
#define _STDLIB_FILE_H

#include <stdio.h>


#define OUTFILE_PATHSIZ 100

struct OutFile {
	char path[OUTFILE_PATHSIZ];
	FILE *out;
};

EXPORTED void die_errno(const char *msg, const char *arg);

EXPORTED struct OutFile *OutFile_xopen(unsigned int i, const char *name);

EXPORTED void OutFile_xclose_and_free(struct OutFile *this);


#endif /* _STDLIB_FILE_H */
