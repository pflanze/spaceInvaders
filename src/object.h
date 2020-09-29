#ifndef _OBJECT_H
#define _OBJECT_H

#include <stdio.h>
#include "utils.h" /* bool */


// Top of the object hierarchy

struct ObjectInterface {
	void(*pp)(const void *this, FILE *out); // pretty-print
};


// Shorter virtual call syntax
#define V(method, o, args...)  (o)->vtable->method((o) , ##args)



#endif // _OBJECT_H
