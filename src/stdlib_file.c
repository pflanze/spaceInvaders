#include <errno.h>
#include <string.h>
#include "stdlib_file.h"
#include "stdlib_utils.h"
#include "pp.h"
#include "perhaps_assert.h"


EXPORTED
void die_errno(const char *msg, const char *arg) {
	fprintf(stderr, "error: %s (%s): %s\n", msg, arg, strerror(errno));
	exit(1);
}



static void
SimpleOutFile_pp(const struct SimpleOutFile *this, FILE *out) {
	PP_PRINTF("(struct SimpleOutFile) {");
	PP_PRINTF(" .out = %p", this->out);
	PP_PRINTF(" .path = \"%s\"", VCALL(path, this)); // needs escaping!
	PP_PRINTF(" }");
}
static void _SimpleOutFile_pp(const void *this, FILE *out) {
	SimpleOutFile_pp(this, out);
}

static const char *
SimpleOutFile_path(const struct SimpleOutFile *this) {
	return this->path;
}
static const char *
_SimpleOutFile_path(const void *this) {
	return SimpleOutFile_path(this);
}

const struct OutFileVTable SimpleOutFile_VTable = {
	.pp = &_SimpleOutFile_pp,
	.path = &_SimpleOutFile_path
};

static void
NumberedOutFile_pp(const struct NumberedOutFile *this, FILE *out) {
	PP_PRINTF("(struct NumberedOutFile) {");
	PP_PRINTF(" .out = %p", this->out);
	PP_PRINTF(" /* , .path reads as \"%s\" */", VCALL(path, this)); // needs escaping, too!
	PP_PRINTF(" }");
}
static void _NumberedOutFile_pp(const void *this, FILE *out) {
	NumberedOutFile_pp(this, out);
}

static const char *
NumberedOutFile_path(const struct NumberedOutFile *this) {
	return this->path;
}
static const char *
_NumberedOutFile_path(const void *this) {
	return NumberedOutFile_path(this);
}

const struct OutFileVTable NumberedOutFile_VTable = {
	.pp = &_NumberedOutFile_pp,
	.path = &_NumberedOutFile_path
};



// base implementation:

static void
_OutFile_xopen(const struct OutFileVTable *const*_this) {
	struct OutFileInterface *this = (void *)_this;
	const char *path = VCALL(path, this);
	this->out = fopen(path, "w");
	if (! this->out) {
		die_errno("open", path);
	}
}
#define OutFile_xopen(this) _OutFile_xopen(&(this)->vtable)

// and the 'constructors':

EXPORTED struct SimpleOutFile *
SimpleOutFile_xopen(const char *path) {
	LET_NEW(this, SimpleOutFile);
	this->path = path;
	OutFile_xopen(this);
	return this;
}

EXPORTED struct NumberedOutFile *
NumberedOutFile_xopen(unsigned int i, const char *name) {
	LET_NEW(this, NumberedOutFile);
	XSNPRINTF(this->path, OUTFILE_PATHSIZ, "%i-%s", i, name);
	OutFile_xopen(this);
	return this;
}

EXPORTED void
_OutFile_xclose_and_free(const struct OutFileVTable *const*_this) {
	struct OutFileInterface *this = (void *)_this;
	if (fclose(this->out) != 0) {
		die_errno("close", VCALL(path, this));
	}
	free(this);
}

