#ifndef _OBJECT_H
#define _OBJECT_H

#include <stdio.h>
#include "utils.h" /* bool */


// Top of the object hierarchy

#define OBJECT_INTERFACE  void(*pp)(const struct ObjectVTable *const*this, FILE *out); // pretty-print

struct ObjectVTable {
	OBJECT_INTERFACE
};

struct ObjectInterface {
	const struct ObjectVTable *vtable;
};


// Shorter virtual call syntax
#define VCALL(method, o, args...)  (o)->vtable->method(&(o)->vtable , ##args)

// Easy heap allocation; if you use this, you have to #include "stdlib_utils.h"
#define LET_NEW(var, T)								\
	LET_XMALLOC(this, struct T);					\
	this->vtable = &T##_VTable;

#endif // _OBJECT_H
