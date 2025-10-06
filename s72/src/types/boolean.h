#ifndef S72_BOOLEAN_H
#define S72_BOOLEAN_H

#include "../s72.h"

// Boolean type structure (singletons don't need instance data)
typedef struct {
    // No instance data needed - true/false are singletons
} S72Boolean;

// Boolean type functions
void s72_boolean_init(void);
bool s72_is_boolean(S72Value val);

// Boolean singleton accessors
S72Value s72_true(void);
S72Value s72_false(void);
S72Value s72_nil(void);

// Boolean methods (native implementations)
oop s72_boolean_if_true(oop closure, oop state, oop receiver, ...);
oop s72_boolean_if_false(oop closure, oop state, oop receiver, ...);
oop s72_boolean_if_true_if_false(oop closure, oop state, oop receiver, ...);
oop s72_boolean_and(oop closure, oop state, oop receiver, ...);
oop s72_boolean_or(oop closure, oop state, oop receiver, ...);
oop s72_boolean_not(oop closure, oop state, oop receiver);

#endif // S72_BOOLEAN_H
