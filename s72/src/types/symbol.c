#include "symbol.h"
#include "string.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// External references
extern struct __libid *_libid;
extern oop s72_symbol_vtable;
extern S72Value S72_TRUE, S72_FALSE, S72_NIL;

// External global selectors
extern oop SEL_PLUS, SEL_MINUS, SEL_MULTIPLY, SEL_DIVIDE;
extern oop SEL_EQUALS, SEL_LESS_THAN, SEL_GREATER_THAN;
extern oop SEL_LESS_EQUAL, SEL_GREATER_EQUAL;
extern oop SEL_PRINT, SEL_VALUE, SEL_TO, SEL_BECOME, SEL_DOES_NOT_UNDERSTAND;

// ============================================================================
// Pure libid Symbol Implementation
// ============================================================================

oop s72_symbol_new_libid(const char *name) {
    if (!s72_symbol_vtable) {
        fprintf(stderr, "Error: Symbol vtable not initialized\n");
        return 0;
    }
    
    if (!name) {
        fprintf(stderr, "Error: Symbol name cannot be null\n");
        return 0;
    }
    
    // Allocate symbol object using libid
    oop symbol_obj = S72_ALLOC(s72_symbol_vtable, sizeof(struct t_Symbol));
    if (!symbol_obj) {
        fprintf(stderr, "Error: Failed to allocate symbol object\n");
        return 0;
    }
    
    Symbol symbol = (Symbol)symbol_obj;
    
    // Intern the symbol name with libid
    oop interned = S72_INTERN(name);
    if (!interned) {
        fprintf(stderr, "Error: Failed to intern symbol name\n");
        return 0;
    }
    
    symbol->interned_oop = interned;
    
    // Store name as String object (not malloc!)
    S72Value name_str = s72_string_new(name);
    if (s72_is_nil(name_str)) {
        fprintf(stderr, "Error: Failed to create symbol name string\n");
        return 0;
    }
    
    symbol->name = name_str.obj;
    
    return symbol_obj;
}

// Symbol creation and testing
S72Value s72_symbol_new(const char *name) {
    if (!name) {
        return S72_NIL;
    }
    
    oop symbol_obj = s72_symbol_new_libid(name);
    if (!symbol_obj) {
        return S72_NIL;
    }
    
    S72Value result = {symbol_obj};
    return result;
}

bool s72_is_symbol(S72Value val) {
    if (s72_is_nil(val)) return false;
    
    // Check if object has the symbol vtable
    if (!val.obj) return false;
    
    // Get the vtable from the libid object
    oop *vtable_ptr = (oop *)val.obj;
    oop obj_vtable = vtable_ptr[-1];
    
    // Compare with symbol vtable
    oop expected_vtable = s72_symbol_vtable->_vtable[-1];
    return (obj_vtable == expected_vtable);
}

const char *s72_symbol_name(S72Value val) {
    if (!s72_is_symbol(val)) {
        s72_error("Value is not a symbol");
        return "";
    }
    
    Symbol symbol = (Symbol)val.obj;
    S72Value name_val = {symbol->name};
    return s72_string_data(name_val);
}

oop s72_symbol_oop(S72Value val) {
    if (!s72_is_symbol(val)) {
        s72_error("Value is not a symbol");
        return NULL;
    }
    
    Symbol symbol = (Symbol)val.obj;
    return symbol->interned_oop;
}

// ============================================================================
// Symbol Methods (Pure libid implementations)
// ============================================================================

oop s72_symbol_print(oop closure, oop state, oop self) {
    (void)closure; (void)state;
    
    Symbol symbol = (Symbol)self;
    S72Value name_val = {symbol->name};
    const char *name = s72_string_data(name_val);
    
    printf("'%s", name);
    return self;
}

oop s72_symbol_equals(oop closure, oop state, oop self, ...) {
    (void)closure; (void)state;
    
    va_list args;
    va_start(args, self);
    oop arg = va_arg(args, oop);
    va_end(args);
    
    if (!arg) {
        s72_error("= requires one argument");
        return S72_FALSE.obj;
    }
    
    S72Value self_val = {self};
    S72Value arg_val = {arg};
    
    if (!s72_is_symbol(self_val)) {
        s72_error("= called on non-symbol");
        return S72_FALSE.obj;
    }
    
    if (!s72_is_symbol(arg_val)) {
        return S72_FALSE.obj;  // Different types are not equal
    }
    
    // Compare the interned oops - symbols with same name should have same interned oop
    Symbol self_sym = (Symbol)self;
    Symbol arg_sym = (Symbol)arg;
    
    bool equal = (self_sym->interned_oop == arg_sym->interned_oop);
    return equal ? S72_TRUE.obj : S72_FALSE.obj;
}

oop s72_symbol_to(oop closure, oop state, oop self, ...) {
    (void)closure; (void)state;
    
    va_list args;
    va_start(args, self);
    oop arg = va_arg(args, oop);
    va_end(args);
    
    if (!arg) {
        s72_error("to: requires one argument");
        return S72_NIL.obj;
    }
    
    // This would implement the "to:" method for creating ranges
    // For now, just return a simple implementation
    printf("Range from ");
    s72_symbol_print(0, 0, self);
    printf(" to ");
    
    S72Value arg_val = {arg};
    if (s72_is_symbol(arg_val)) {
        s72_symbol_print(0, 0, arg);
    } else {
        printf("<object>");
    }
    
    return self;
}

// ============================================================================
// Symbol Table Management
// ============================================================================

void s72_symbol_intern_selectors(void) {
    // Intern common selectors used by the system and assign to global variables
    // This ensures they're available for method dispatch

    SEL_PLUS = S72_INTERN("+");
    SEL_MINUS = S72_INTERN("-");
    SEL_MULTIPLY = S72_INTERN("*");
    SEL_DIVIDE = S72_INTERN("/");
    SEL_EQUALS = S72_INTERN("=");
    SEL_LESS_THAN = S72_INTERN("<");
    SEL_GREATER_THAN = S72_INTERN(">");
    SEL_LESS_EQUAL = S72_INTERN("<=");
    SEL_GREATER_EQUAL = S72_INTERN(">=");
    SEL_PRINT = S72_INTERN("print");
    SEL_VALUE = S72_INTERN("value");
    SEL_TO = S72_INTERN("to:");
    SEL_BECOME = S72_INTERN("become:");
    SEL_DOES_NOT_UNDERSTAND = S72_INTERN("doesNotUnderstand:");

    // Additional selectors for other object types
    S72_INTERN("print:");
    S72_INTERN("at:");
    S72_INTERN("at:put:");
    S72_INTERN("size");
    S72_INTERN("length");
    S72_INTERN("forward:");
    S72_INTERN("turn:");
    S72_INTERN("penUp");
    S72_INTERN("penDown");
    S72_INTERN("clear");
    S72_INTERN("show");
    S72_INTERN("position");
    S72_INTERN("heading");

    // Verify all critical selectors were interned
    if (!SEL_PLUS || !SEL_MINUS || !SEL_MULTIPLY || !SEL_DIVIDE ||
        !SEL_EQUALS || !SEL_LESS_THAN || !SEL_GREATER_THAN ||
        !SEL_LESS_EQUAL || !SEL_GREATER_EQUAL || !SEL_PRINT || !SEL_VALUE ||
        !SEL_TO || !SEL_BECOME || !SEL_DOES_NOT_UNDERSTAND) {
        s72_error("Failed to intern core selectors");
    }

    // Common selectors interned successfully
}

// ============================================================================
// Initialization
// ============================================================================

void s72_symbol_init(void) {
    // Vtable should already be created in main.c
    if (!s72_symbol_vtable) {
        fprintf(stderr, "Error: Symbol vtable not initialized\n");
        return;
    }
    
    // Install symbol methods
    oop sel_print = S72_INTERN("print");
    oop sel_equals = S72_INTERN("=");
    oop sel_to = S72_INTERN("to:");
    
    S72_METHOD(s72_symbol_vtable, sel_print, s72_symbol_print);
    S72_METHOD(s72_symbol_vtable, sel_equals, s72_symbol_equals);
    S72_METHOD(s72_symbol_vtable, sel_to, s72_symbol_to);
    
    // Intern common selectors
    s72_symbol_intern_selectors();
    
    // Pure libid symbol system initialized successfully
}
