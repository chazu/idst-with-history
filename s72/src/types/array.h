#ifndef S72_ARRAY_H
#define S72_ARRAY_H

#include "../s72.h"

// Forward declaration for circular reference
struct S72Value;

// Array type structure (fixed-size array)
typedef struct {
    S72Value *elements;     // Array of elements
    int size;               // Number of elements
    int capacity;           // Allocated capacity (for growth)
} S72Array;

// Array type functions
void s72_array_init(void);
S72Value s72_array_new(int size);
S72Value s72_array_new_with_values(int size, S72Value *values);
bool s72_is_array(S72Value val);

// Array access functions
int s72_array_size(S72Value array);
S72Value s72_array_at(S72Value array, int index);
void s72_array_at_put(S72Value array, int index, S72Value value);

// Array iteration
void s72_array_each(S72Value array, S72Value block);

// Array method implementations (native C functions for libid)
oop s72_array_size_method(oop closure, oop state, oop receiver);
oop s72_array_at_method(oop closure, oop state, oop receiver, oop index);
oop s72_array_at_put_method(oop closure, oop state, oop receiver, oop index, oop value);
oop s72_array_each_method(oop closure, oop state, oop receiver, oop block);

// Array vtable
extern oop s72_array_vtable;

#endif // S72_ARRAY_H
