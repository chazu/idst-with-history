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
    _libid_method(s72_symbol_vtable, SEL_PRINT, (_imp_t)s72_symbol_print);
    _libid_method(s72_symbol_vtable, SEL_EQUALS, (_imp_t)s72_symbol_equals);
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
    extern oop s72_symbol_proto;
    oop sym_oop = S72_ALLOC(s72_symbol_proto, sizeof(S72Symbol));
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
    if (s72_is_nil(val)) return false;
    
    // Check if the object's vtable matches symbol vtable
    oop vtable = val.obj->_vtable[-1];
    return vtable == s72_symbol_vtable;
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
    if (!s72_is_symbol(val)) {
        s72_error("Value is not a symbol");
        return NULL;
    }
    
    S72Symbol *sym = (S72Symbol *)val.obj;
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
    SEL_PLUS = S72_INTERN("+");
    SEL_MINUS = S72_INTERN("-");
    SEL_MULTIPLY = S72_INTERN("*");
    SEL_DIVIDE = S72_INTERN("/");
    SEL_EQUALS = S72_INTERN("=");
    SEL_PRINT = S72_INTERN("print");
    SEL_VALUE = S72_INTERN("value");
    
    if (!SEL_PLUS || !SEL_MINUS || !SEL_MULTIPLY || !SEL_DIVIDE || 
        !SEL_EQUALS || !SEL_PRINT || !SEL_VALUE) {
        s72_error("Failed to intern core selectors");
    }
}
