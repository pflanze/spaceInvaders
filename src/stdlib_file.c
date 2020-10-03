#include <errno.h>
#include <string.h>
#include "stdlib_file.h"
#include "stdlib_utils.h"
#include "pp.h"


EXPORTED
void die_errno(const char *msg, const char *arg) {
	fprintf(stderr, "error: %s (%s): %s\n", msg, arg, strerror(errno));
	exit(1);
}



static void
SimpleOutFile_pp(const struct SimpleOutFile *this, FILE *out) {
	PP_PRINTF("(struct SimpleOutFile) {");
	PP_PRINTF(" .out = %p",
#ifdef DIRECT
			  this->super.out
#else
			  VCALL(out, this)
#endif
		);
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

#ifndef DIRECT
static FILE *
SimpleOutFile_out(const struct SimpleOutFile *this) {
	return this->out;
}
static FILE *
_SimpleOutFile_out(const void *this) {
	return SimpleOutFile_out(this);
}
#endif

const struct OutFileVTable SimpleOutFile_OutFileVTable = {
	.pp = &_SimpleOutFile_pp
	, .path = &_SimpleOutFile_path
#ifndef DIRECT
	, .out = &_SimpleOutFile_out
#endif
};

static void
NumberedOutFile_pp(const struct NumberedOutFile *this, FILE *out) {
	PP_PRINTF("(struct NumberedOutFile) {");
	PP_PRINTF(" .out = %p",
#ifdef DIRECT
			  this->super.out
#else
			  VCALL(out, this)
#endif
		);
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

#ifndef DIRECT
static FILE *
NumberedOutFile_out(const struct NumberedOutFile *this) {
	return this->out;
}
static FILE *
_NumberedOutFile_out(const void *this) {
	return NumberedOutFile_out(this);
}
#endif

const struct OutFileVTable NumberedOutFile_OutFileVTable = {
	.pp = &_NumberedOutFile_pp
	, .path = &_NumberedOutFile_path
#ifndef DIRECT
	, .out = &_NumberedOutFile_out
#endif
};





// and the 'constructors':

EXPORTED struct SimpleOutFile *
SimpleOutFile_xopen(const char *path) {
	LET_XMALLOC(this, struct SimpleOutFile);
	this->vtable = &SimpleOutFile_OutFileVTable;
	this->path = path;
#ifdef DIRECT
	this->super.out = fopen(this->path, "w");
	if (! this->super.out) {
#else
	this->out = fopen(this->path, "w");
	if (! this->out) {
#endif
		die_errno("open", this->path);
	}
	return this;
}

EXPORTED struct NumberedOutFile *
NumberedOutFile_xopen(unsigned int i, const char *name) {
	LET_XMALLOC(this, struct NumberedOutFile);
	this->vtable = &NumberedOutFile_OutFileVTable;
	snprintf(this->path, OUTFILE_PATHSIZ, "%i-%s", i, name);
#ifdef DIRECT
	this->super.out = fopen(this->path, "w");
	if (! this->super.out) {
#else
	this->out = fopen(this->path, "w");
	if (! this->out) {
#endif
		die_errno("open", this->path);
	}
	return this;
}

EXPORTED void
OutFile_xclose_and_free(struct OutFileInterface *this) {
#ifdef DIRECT
	if (fclose(this->super.out) != 0) {
		die_errno("close", VCALL(path, this));
	}
#else
	if (fclose(VCALL(out, this)) != 0) {
		die_errno("close", VCALL(path, this));
	}
#endif
	free(this);
}

