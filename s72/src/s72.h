#ifndef S72_H
#define S72_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Include enhanced libid
#include <id/id.h>
#include <id/libid_enhanced.h>

// Complete libid object structure (from libid.c)
struct t__object {
    union {
        oop _vtable[0];
        // Other union members omitted for simplicity
    };
};

// Forward declaration for libid initialization
struct __libid *_libid_init(int *argcp, char ***argvp, char ***envp);

// Forward declarations
typedef struct S72Object S72Object;
typedef struct S72Value S72Value;
typedef struct S72Env S72Env;

// S72 Value type - wraps libid oop
typedef struct S72Value {
    oop obj;  // libid object pointer
} S72Value;

// Value type tags (stored in libid vtable)
typedef enum {
    S72_TYPE_NIL,
    S72_TYPE_BOOLEAN,
    S72_TYPE_NUMBER,
    S72_TYPE_STRING,
    S72_TYPE_SYMBOL,
    S72_TYPE_LIST,
    S72_TYPE_BLOCK,
    S72_TYPE_OBJECT
} S72Type;

// Global libid instance
extern struct __libid *_libid;

// Global selectors (interned once)
extern oop SEL_PLUS;
extern oop SEL_MINUS;
extern oop SEL_MULTIPLY;
extern oop SEL_DIVIDE;
extern oop SEL_EQUALS;
extern oop SEL_PRINT;
extern oop SEL_VALUE;

// Global singletons
extern S72Value S72_NIL;
extern S72Value S72_TRUE;
extern S72Value S72_FALSE;

// Global vtables for each type
extern oop s72_nil_vtable;
extern oop s72_boolean_vtable;
extern oop s72_number_vtable;
extern oop s72_string_vtable;
extern oop s72_symbol_vtable;
extern oop s72_list_vtable;
extern oop s72_block_vtable;
extern oop s72_object_vtable;

// Core API functions
void s72_init(int *argc, char ***argv, char ***envp);
void s72_shutdown(void);

// Value creation and testing
S72Value s72_make_nil(void);
S72Value s72_make_boolean(bool value);
S72Value s72_make_number(double value);
S72Value s72_make_string(const char *str);
S72Value s72_make_symbol(const char *name);

bool s72_is_nil(S72Value val);
bool s72_is_boolean(S72Value val);
bool s72_is_number(S72Value val);
bool s72_is_string(S72Value val);
bool s72_is_symbol(S72Value val);

// Value extraction
bool s72_as_boolean(S72Value val);
double s72_as_number(S72Value val);
const char *s72_as_string(S72Value val);
const char *s72_as_symbol(S72Value val);

// Message sending
S72Value s72_send(S72Value receiver, oop selector, int argc, S72Value *argv);

// Printing
void s72_print_value(S72Value val);
char *s72_value_to_string(S72Value val);

// Error handling
void s72_error(const char *fmt, ...);
void s72_type_error(const char *expected, S72Value got);

// Memory management helpers - use _libid pointer members
#define S72_ALLOC(type, size) _libid->alloc(type, size)
#define S72_INTERN(str) _libid->intern(str)
#define S72_PROTO(base) _libid->proto(base)
#define S72_METHOD(type, sel, meth) _libid->method(type, sel, (_imp_t)meth)

// Dispatch helpers using enhanced libid macros
#define S72_SEND0(sel, rcv) _sendv0(sel, (rcv).obj)
#define S72_SEND1(sel, rcv, a1) _sendv1(sel, (rcv).obj, (a1).obj)
#define S72_SEND2(sel, rcv, a1, a2) _sendv2(sel, (rcv).obj, (a1).obj, (a2).obj)

#endif // S72_H
