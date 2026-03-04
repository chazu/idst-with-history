#ifndef SYMBOL_H
#define SYMBOL_H

#include "../s72.h"

// Symbol object structure (simplified for M0)
typedef struct S72Symbol {
    oop interned_oop;  // libid interned symbol
    char *name;        // Copy of symbol name for easy access
} S72Symbol;

// Symbol type initialization
void s72_symbol_init(void);

// Symbol creation and testing
S72Value s72_symbol_new(const char *name);
bool s72_is_symbol(S72Value val);
const char *s72_symbol_name(S72Value val);
oop s72_symbol_oop(S72Value val);  // Get the underlying libid oop

// Symbol methods (native implementations)
oop s72_symbol_print(struct __send *send, oop self, oop receiver);
oop s72_symbol_equals(struct __send *send, oop self, oop receiver, oop arg);
oop s72_symbol_to(oop closure, oop state, oop receiver, ...);

// Symbol table management
void s72_symbol_intern_selectors(void);

#endif // SYMBOL_H
