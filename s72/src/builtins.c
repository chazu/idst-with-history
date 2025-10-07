#include "builtins.h"
#include "eval.h"
#include "types/transcript.h"
#include "types/number.h"
#include "types/boolean.h"
#include <stdio.h>

// External references
extern struct __libid *_libid;
extern S72Value S72_NIL, S72_TRUE, S72_FALSE;
extern oop s72_global_env;

// Built-in function implementations

oop s72_builtin_show_(oop closure, oop state, oop receiver, ...) {
    (void)closure; (void)state; (void)receiver; // Unused
    va_list args;
    va_start(args, receiver);
    oop value = va_arg(args, oop);
    va_end(args);
    
    // Print the value using transcript
    S72Value transcript = s72_transcript_singleton();
    oop print_selector = _libid->intern("print:");
    S72_METHOD_CALL(transcript.obj, print_selector, value);
    
    // Print newline
    oop cr_selector = _libid->intern("cr");
    S72_METHOD_CALL(transcript.obj, cr_selector);
    
    return value;
}

oop s72_builtin_println_(oop closure, oop state, oop receiver, ...) {
    (void)closure; (void)state; (void)receiver; // Unused
    va_list args;
    va_start(args, receiver);
    oop value = va_arg(args, oop);
    va_end(args);
    
    // Same as show:
    return s72_builtin_show_(closure, state, receiver, value);
}

oop s72_builtin_def_(oop closure, oop state, oop receiver, ...) {
    (void)closure; (void)state; (void)receiver; // Unused
    va_list args;
    va_start(args, receiver);
    oop name_obj = va_arg(args, oop);
    oop value_obj = va_arg(args, oop);
    va_end(args);
    
    // Extract symbol name
    if (!name_obj) {
        fprintf(stderr, "Error: def expects a symbol name\n");
        return S72_NIL.obj;
    }
    
    // For now, just return the value (global binding not implemented)
    printf("DEBUG: def called with name=%p, value=%p\n", (void*)name_obj, (void*)value_obj);
    return value_obj;
}

oop s72_builtin_not_(oop closure, oop state, oop receiver, ...) {
    (void)closure; (void)state; (void)receiver; // Unused
    va_list args;
    va_start(args, receiver);
    oop bool_obj = va_arg(args, oop);
    va_end(args);
    
    if (bool_obj == S72_TRUE.obj) {
        return S72_FALSE.obj;
    } else if (bool_obj == S72_FALSE.obj) {
        return S72_TRUE.obj;
    } else {
        // Non-boolean values are considered true
        return S72_FALSE.obj;
    }
}

oop s72_builtin_and_(oop closure, oop state, oop receiver, ...) {
    (void)closure; (void)state; (void)receiver; // Unused
    va_list args;
    va_start(args, receiver);
    oop a_obj = va_arg(args, oop);
    oop b_obj = va_arg(args, oop);
    va_end(args);
    
    if (a_obj == S72_FALSE.obj || a_obj == S72_NIL.obj) {
        return S72_FALSE.obj;
    } else if (b_obj == S72_FALSE.obj || b_obj == S72_NIL.obj) {
        return S72_FALSE.obj;
    } else {
        return S72_TRUE.obj;
    }
}

oop s72_builtin_or_(oop closure, oop state, oop receiver, ...) {
    (void)closure; (void)state; (void)receiver; // Unused
    va_list args;
    va_start(args, receiver);
    oop a_obj = va_arg(args, oop);
    oop b_obj = va_arg(args, oop);
    va_end(args);
    
    if (a_obj != S72_FALSE.obj && a_obj != S72_NIL.obj) {
        return S72_TRUE.obj;
    } else if (b_obj != S72_FALSE.obj && b_obj != S72_NIL.obj) {
        return S72_TRUE.obj;
    } else {
        return S72_FALSE.obj;
    }
}

oop s72_builtin_abs_(oop closure, oop state, oop receiver, ...) {
    (void)closure; (void)state; (void)receiver; // Unused
    va_list args;
    va_start(args, receiver);
    oop num_obj = va_arg(args, oop);
    va_end(args);
    
    S72Value num_val = { .obj = num_obj };
    if (!s72_is_number(num_val)) {
        fprintf(stderr, "Error: abs: expects a number\n");
        return S72_NIL.obj;
    }
    
    double value = s72_number_value(num_val);
    double result = (value < 0) ? -value : value;
    
    return s72_number_new(result).obj;
}

oop s72_builtin_min_(oop closure, oop state, oop receiver, ...) {
    (void)closure; (void)state; (void)receiver; // Unused
    va_list args;
    va_start(args, receiver);
    oop a_obj = va_arg(args, oop);
    oop b_obj = va_arg(args, oop);
    va_end(args);
    
    S72Value a_val = { .obj = a_obj };
    S72Value b_val = { .obj = b_obj };
    
    if (!s72_is_number(a_val) || !s72_is_number(b_val)) {
        fprintf(stderr, "Error: min: expects two numbers\n");
        return S72_NIL.obj;
    }
    
    double a = s72_number_value(a_val);
    double b = s72_number_value(b_val);
    double result = (a < b) ? a : b;
    
    return s72_number_new(result).obj;
}

oop s72_builtin_max_(oop closure, oop state, oop receiver, ...) {
    (void)closure; (void)state; (void)receiver; // Unused
    va_list args;
    va_start(args, receiver);
    oop a_obj = va_arg(args, oop);
    oop b_obj = va_arg(args, oop);
    va_end(args);
    
    S72Value a_val = { .obj = a_obj };
    S72Value b_val = { .obj = b_obj };
    
    if (!s72_is_number(a_val) || !s72_is_number(b_val)) {
        fprintf(stderr, "Error: max: expects two numbers\n");
        return S72_NIL.obj;
    }
    
    double a = s72_number_value(a_val);
    double b = s72_number_value(b_val);
    double result = (a > b) ? a : b;
    
    return s72_number_new(result).obj;
}

oop s72_builtin_square_(oop closure, oop state, oop receiver, ...) {
    (void)closure; (void)state; (void)receiver; // Unused
    va_list args;
    va_start(args, receiver);
    oop num_obj = va_arg(args, oop);
    va_end(args);
    
    S72Value num_val = { .obj = num_obj };
    if (!s72_is_number(num_val)) {
        fprintf(stderr, "Error: square: expects a number\n");
        return S72_NIL.obj;
    }
    
    double value = s72_number_value(num_val);
    double result = value * value;
    
    return s72_number_new(result).obj;
}

// Installation function
void s72_install_builtins(void) {
    if (!s72_global_env) {
        fprintf(stderr, "Error: Cannot install builtins - global environment not initialized\n");
        return;
    }
    
    // Install built-in functions in global environment
    oop show_selector = _libid->intern("show:");
    oop println_selector = _libid->intern("println:");
    oop def_selector = _libid->intern("def");
    oop not_selector = _libid->intern("not:");
    oop and_selector = _libid->intern("and:");
    oop or_selector = _libid->intern("or:");
    oop abs_selector = _libid->intern("abs:");
    oop min_selector = _libid->intern("min:");
    oop max_selector = _libid->intern("max:");
    oop square_selector = _libid->intern("square:");
    
    // Create function objects and bind them
    // For now, we'll bind them as global functions
    // TODO: Implement proper function object creation
    
    printf("DEBUG: Built-in functions installed\n");
}
