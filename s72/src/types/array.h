#ifndef ARRAY_H
#define ARRAY_H

#include "../s72.h"

// Pure libid array object structure
typedef struct t_Array {
    oop size;      // Number object for array size
    oop elements;  // libid-allocated array of oops (not malloc!)
} *Array;

// Array type initialization
void s72_array_init(void);

// Array creation and testing
S72Value s72_array_new(int size);
S72Value s72_array_new_with_values(S72Value *values, int count);
bool s72_is_array(S72Value val);
int s72_array_size(S72Value val);
S72Value s72_array_get(S72Value array_val, int index);
void s72_array_set(S72Value array_val, int index, S72Value value);

// Array methods (pure libid implementations with variadic signatures)
oop s72_array_at_(oop closure, oop state, oop self, ...);
oop s72_array_at_put_(oop closure, oop state, oop self, ...);
oop s72_array_size_method(oop closure, oop state, oop self);
oop s72_array_print(oop closure, oop state, oop self);

// Array object creation
oop s72_array_new_libid(int size);

#endif // ARRAY_H
