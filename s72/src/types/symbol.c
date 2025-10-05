#include "symbol.h"
#include <stdio.h>
#include <string.h>

// Global vtable for symbols
extern oop s72_symbol_vtable;

// Symbol type initialization
void s72_symbol_init(void) {
    if (!s72_symbol_vtable) {
        s72_error("Symbol vtable not initialized");
        return;
    }

    // Install native methods
    S72_METHOD(s72_symbol_vtable, SEL_PRINT, s72_symbol_print);
    S72_METHOD(s72_symbol_vtable, SEL_EQUALS, s72_symbol_equals);
}

// Symbol creation and testing
S72Value s72_symbol_new(const char *name) {
    if (!name) {
        return S72_NIL;
    }

    // Intern the symbol using libid
    oop interned = S72_INTERN(name);
    if (!interned) {
        s72_error("Failed to intern symbol");
        return S72_NIL;
    }

    // Create S72 symbol wrapper
    oop sym_oop = S72_ALLOC(s72_symbol_vtable, sizeof(S72Symbol));
    if (!sym_oop) {
        s72_error("Failed to allocate symbol");
        return S72_NIL;
    }

    S72Symbol *sym = (S72Symbol *)sym_oop;
    sym->interned_oop = interned;
    sym->name = strdup(name);  // Keep a copy for easy access

    S72Value result = {sym_oop};
    return result;
}

bool s72_is_symbol(S72Value val) {
    if (s72_is_nil(val)) {
        return false;
    }

    // For M0, we'll use a simple heuristic:
    // Check if this looks like a symbol object
    S72Symbol *sym = (S72Symbol *)val.obj;
    return sym && sym->name && sym->interned_oop;
}

const char *s72_symbol_name(S72Value val) {
    if (!s72_is_symbol(val)) {
        s72_error("Value is not a symbol");
        return "";
    }

    S72Symbol *sym = (S72Symbol *)val.obj;
    return sym->name ? sym->name : "";
}

oop s72_symbol_oop(S72Value val) {
    printf("DEBUG: s72_symbol_oop called with val.obj=%p\n", val.obj);

    if (!s72_is_symbol(val)) {
        s72_error("Value is not a symbol");
        return NULL;
    }

    S72Symbol *sym = (S72Symbol *)val.obj;
    printf("DEBUG: sym=%p, sym->interned_oop=%p\n", sym, sym->interned_oop);
    return sym->interned_oop;
}

// Symbol methods

oop s72_symbol_print(struct __send *send, oop self, oop receiver) {
    S72Value recv_val = {receiver};
    
    if (!s72_is_symbol(recv_val)) {
        s72_error("print called on non-symbol");
        return NULL;
    }
    
    const char *name = s72_symbol_name(recv_val);
    printf("'%s", name);
    
    return receiver;  // Return self
}

oop s72_symbol_equals(struct __send *send, oop self, oop receiver, oop arg) {
    if (!arg) {
        s72_error("= requires one argument");
        return NULL;
    }
    
    S72Value recv_val = {receiver};
    S72Value arg_val = {arg};
    
    if (!s72_is_symbol(recv_val)) {
        s72_error("= called on non-symbol");
        return NULL;
    }
    
    if (!s72_is_symbol(arg_val)) {
        return S72_FALSE.obj;  // Different types are not equal
    }
    
    // Symbols are interned, so we can compare by pointer equality
    oop a = s72_symbol_oop(recv_val);
    oop b = s72_symbol_oop(arg_val);
    
    bool equal = (a == b);
    return equal ? S72_TRUE.obj : S72_FALSE.obj;
}

// Symbol table management
void s72_symbol_intern_selectors(void) {
    // Intern common selectors used throughout the system
    SEL_PLUS = _libid->intern("+");
    SEL_MINUS = _libid->intern("-");
    SEL_MULTIPLY = _libid->intern("*");
    SEL_DIVIDE = _libid->intern("/");
    SEL_EQUALS = _libid->intern("=");
    SEL_PRINT = _libid->intern("print");
    SEL_VALUE = _libid->intern("value");

    printf("DEBUG: Interned selectors - PLUS=%p, MINUS=%p, MULTIPLY=%p\n",
           SEL_PLUS, SEL_MINUS, SEL_MULTIPLY);

    if (!SEL_PLUS || !SEL_MINUS || !SEL_MULTIPLY || !SEL_DIVIDE ||
        !SEL_EQUALS || !SEL_PRINT || !SEL_VALUE) {
        s72_error("Failed to intern core selectors");
    }
}
