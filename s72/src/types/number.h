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
oop s72_number_add(struct __send *send, oop self, oop receiver, oop arg);
oop s72_number_subtract(struct __send *send, oop self, oop receiver, oop arg);
oop s72_number_multiply(struct __send *send, oop self, oop receiver, oop arg);
oop s72_number_divide(struct __send *send, oop self, oop receiver, oop arg);
oop s72_number_equals(struct __send *send, oop self, oop receiver, oop arg);

// Number comparison methods
oop s72_number_less_than(struct __send *send, oop self, oop receiver, oop arg);
oop s72_number_greater_than(struct __send *send, oop self, oop receiver, oop arg);
oop s72_number_less_equal(struct __send *send, oop self, oop receiver, oop arg);
oop s72_number_greater_equal(struct __send *send, oop self, oop receiver, oop arg);

// Number printing
oop s72_number_print(struct __send *send, oop self, oop receiver);

#endif // NUMBER_H
