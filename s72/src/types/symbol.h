#ifndef SYMBOL_H
#define SYMBOL_H

#include "../s72.h"

// Pure libid symbol object structure
typedef struct t_Symbol {
    oop interned_oop;  // libid interned symbol
    oop name;          // String object containing symbol name (not malloc!)
} *Symbol;

// Symbol type initialization
void s72_symbol_init(void);

// Symbol creation and testing
S72Value s72_symbol_new(const char *name);
bool s72_is_symbol(S72Value val);
const char *s72_symbol_name(S72Value val);
oop s72_symbol_oop(S72Value val);  // Get the underlying libid oop

// Symbol methods (pure libid implementations with variadic signatures)
oop s72_symbol_print(oop closure, oop state, oop self);
oop s72_symbol_equals(oop closure, oop state, oop self, ...);
oop s72_symbol_to(oop closure, oop state, oop self, ...);

// Symbol table management
void s72_symbol_intern_selectors(void);

// Symbol object creation
oop s72_symbol_new_libid(const char *name);

#endif // SYMBOL_H
