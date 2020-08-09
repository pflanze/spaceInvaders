#ifndef _OBJECT_H
#define _OBJECT_H

#include "utils.h" /* bool */


// Top of the object hierarchy

struct ObjectInterface {
    void(*pp)(void* this); // pretty-print
};


// Shorter virtual call syntax
#define V(method, o, args...)  (o)->vtable->method((o) , ##args)


// Helper functions for pp

const char* bool_show(bool v);


#endif // _OBJECT_H
