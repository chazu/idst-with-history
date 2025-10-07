#ifndef S72_BUILTINS_H
#define S72_BUILTINS_H

#include "s72.h"
#include <stdarg.h>

// Built-in function declarations
oop s72_builtin_show_(oop closure, oop state, oop receiver, ...);
oop s72_builtin_println_(oop closure, oop state, oop receiver, ...);
oop s72_builtin_def_(oop closure, oop state, oop receiver, ...);
oop s72_builtin_not_(oop closure, oop state, oop receiver, ...);
oop s72_builtin_and_(oop closure, oop state, oop receiver, ...);
oop s72_builtin_or_(oop closure, oop state, oop receiver, ...);
oop s72_builtin_abs_(oop closure, oop state, oop receiver, ...);
oop s72_builtin_min_(oop closure, oop state, oop receiver, ...);
oop s72_builtin_max_(oop closure, oop state, oop receiver, ...);
oop s72_builtin_square_(oop closure, oop state, oop receiver, ...);

// Installation function
void s72_install_builtins(void);

#endif // S72_BUILTINS_H
