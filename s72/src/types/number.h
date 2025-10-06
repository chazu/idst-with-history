#ifndef NUMBER_H
#define NUMBER_H

#include "../s72.h"

// Number object structure
typedef struct S72Number {
    double value;
} S72Number;

// Number type initialization
void s72_number_init(void);

// Number creation and testing
S72Value s72_number_new(double value);
bool s72_is_number(S72Value val);
double s72_number_value(S72Value val);

// Number arithmetic methods (native implementations)
// Note: _send1 calls with (oop closure, oop state, oop receiver, ...) - bootstrap calling convention with variadic args
oop s72_number_add(oop closure, oop state, oop receiver, ...);
oop s72_number_subtract(oop closure, oop state, oop receiver, ...);
oop s72_number_multiply(oop closure, oop state, oop receiver, ...);
oop s72_number_divide(oop closure, oop state, oop receiver, ...);
oop s72_number_equals(oop closure, oop state, oop receiver, ...);

// Number comparison methods
oop s72_number_less_than(oop closure, oop state, oop receiver, ...);
oop s72_number_greater_than(oop closure, oop state, oop receiver, ...);
oop s72_number_less_equal(oop closure, oop state, oop receiver, ...);
oop s72_number_greater_equal(oop closure, oop state, oop receiver, ...);

// Number printing
// Note: _send0 calls with (oop closure, oop state, oop receiver)
oop s72_number_print(oop closure, oop state, oop receiver);

#endif // NUMBER_H
