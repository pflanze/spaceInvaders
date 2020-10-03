#ifndef _OBJECT_H
#define _OBJECT_H

#include <stdio.h>
#include "utils.h" /* bool */


// Top of the object hierarchy

#define OBJECT_INTERFACE  void(*pp)(const void *this, FILE *out); // pretty-print

struct ObjectInterface {
	OBJECT_INTERFACE
};


// Shorter virtual call syntax
#define VCALL(method, o, args...)  (o)->vtable->method((o) , ##args)



#endif // _OBJECT_H
