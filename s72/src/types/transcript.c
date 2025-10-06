#include "transcript.h"
#include "string.h"
#include "number.h"
#include "symbol.h"
#include "boolean.h"
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

// External references
extern struct __libid *_libid;
extern oop s72_object_vtable;

// Global transcript singleton
static S72Value S72_TRANSCRIPT;
static oop s72_transcript_vtable = NULL;

// Global selectors for transcript operations
static oop SEL_TRANSCRIPT_PRINT = NULL;
static oop SEL_TRANSCRIPT_CR = NULL;
static oop SEL_TRANSCRIPT_SHOW = NULL;

// Transcript type initialization
void s72_transcript_init(void) {
    // Create transcript vtable
    s72_transcript_vtable = S72_PROTO(s72_object_vtable);
    if (!s72_transcript_vtable) {
        s72_error("Failed to create transcript vtable");
        return;
    }

    // Create the singleton transcript object
    oop transcript_oop = S72_ALLOC(s72_transcript_vtable, sizeof(S72Transcript));
    if (!transcript_oop) {
        s72_error("Failed to create transcript singleton");
        return;
    }
    
    S72_TRANSCRIPT.obj = transcript_oop;

    // Intern selectors
    SEL_TRANSCRIPT_PRINT = _libid->intern("print:");
    SEL_TRANSCRIPT_CR = _libid->intern("cr");
    SEL_TRANSCRIPT_SHOW = _libid->intern("show:");

    // Install methods on transcript vtable
    S72_METHOD(s72_transcript_vtable, SEL_TRANSCRIPT_PRINT, s72_transcript_print);
    S72_METHOD(s72_transcript_vtable, SEL_TRANSCRIPT_CR, s72_transcript_cr);
    S72_METHOD(s72_transcript_vtable, SEL_TRANSCRIPT_SHOW, s72_transcript_show);

    printf("DEBUG: Installed transcript methods on vtable %p\n", s72_transcript_vtable);
}

// Get the transcript singleton
S72Value s72_transcript_singleton(void) {
    return S72_TRANSCRIPT;
}

// Transcript method implementations

// print: - print a value without newline
oop s72_transcript_print(oop closure, oop state, oop receiver, ...) {
    va_list args;
    va_start(args, receiver);
    oop value = va_arg(args, oop);
    va_end(args);

    printf("DEBUG: s72_transcript_print called - value=%p\n", value);

    if (!value) {
        printf("nil");
    } else {
        S72Value val = {value};
        
        if (s72_is_number(val)) {
            double num = s72_as_number(val);
            if (num == floor(num) && num >= -1e15 && num <= 1e15) {
                printf("%.0f", num);
            } else {
                printf("%.15g", num);
            }
        } else if (s72_is_string(val)) {
            printf("%s", s72_as_string(val));  // Print string without quotes
        } else if (s72_is_symbol(val)) {
            printf("%s", s72_as_symbol(val));  // Print symbol without quote
        } else if (s72_is_boolean(val)) {
            printf("%s", s72_as_boolean(val) ? "true" : "false");
        } else {
            printf("<object>");
        }
    }

    fflush(stdout);
    return receiver;  // Return self
}

// cr - print a carriage return (newline)
oop s72_transcript_cr(oop closure, oop state, oop receiver) {
    printf("DEBUG: s72_transcript_cr called\n");
    printf("\n");
    fflush(stdout);
    return receiver;  // Return self
}

// show: - print a value with its representation (like print: but with quotes for strings)
oop s72_transcript_show(oop closure, oop state, oop receiver, ...) {
    va_list args;
    va_start(args, receiver);
    oop value = va_arg(args, oop);
    va_end(args);

    printf("DEBUG: s72_transcript_show called - value=%p\n", value);

    if (!value) {
        printf("nil");
    } else {
        S72Value val = {value};
        
        if (s72_is_number(val)) {
            double num = s72_as_number(val);
            if (num == floor(num) && num >= -1e15 && num <= 1e15) {
                printf("%.0f", num);
            } else {
                printf("%.15g", num);
            }
        } else if (s72_is_string(val)) {
            printf("\"%s\"", s72_as_string(val));  // Print string with quotes
        } else if (s72_is_symbol(val)) {
            printf("'%s", s72_as_symbol(val));  // Print symbol with quote
        } else if (s72_is_boolean(val)) {
            printf("%s", s72_as_boolean(val) ? "true" : "false");
        } else {
            printf("<object>");
        }
    }

    fflush(stdout);
    return receiver;  // Return self
}
