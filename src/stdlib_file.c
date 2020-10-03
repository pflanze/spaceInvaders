#include <errno.h>
#include <string.h>
#include "stdlib_file.h"
#include "stdlib_utils.h"


EXPORTED
void die_errno(const char *msg, const char *arg) {
	fprintf(stderr, "error: %s (%s): %s\n", msg, arg, strerror(errno));
	exit(1);
}


EXPORTED
struct OutFile *OutFile_xopen(unsigned int i, const char *name) {
	LET_XMALLOC(this, struct OutFile);
	snprintf(this->path, OUTFILE_PATHSIZ, "%i-%s", i, name);
	this->out = fopen(this->path, "w");
	if (! this->out) {
		die_errno("open", this->path);
	}
	return this;
}

EXPORTED
void OutFile_xclose_and_free(struct OutFile *this) {
	if (fclose(this->out) != 0) {
		die_errno("close", this->path);
	}
	free(this);
}

