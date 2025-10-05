#include "s72.h"
#include "reader.h"
#include "eval.h"
#include "object.h"
#include "types/number.h"
#include "types/string.h"
#include "types/symbol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

// Global libid instance
struct __libid _libid;

// Global selectors
oop SEL_PLUS = NULL;
oop SEL_MINUS = NULL;
oop SEL_MULTIPLY = NULL;
oop SEL_DIVIDE = NULL;
oop SEL_EQUALS = NULL;
oop SEL_PRINT = NULL;
oop SEL_VALUE = NULL;

// Global singletons
S72Value S72_NIL;
S72Value S72_TRUE;
S72Value S72_FALSE;

// Global vtables
oop s72_nil_vtable = NULL;
oop s72_boolean_vtable = NULL;
oop s72_number_vtable = NULL;
oop s72_string_vtable = NULL;
oop s72_symbol_vtable = NULL;
oop s72_list_vtable = NULL;
oop s72_block_vtable = NULL;
oop s72_object_vtable = NULL;

// Global prototype objects (for allocation)
oop s72_number_proto = NULL;
oop s72_string_proto = NULL;
oop s72_symbol_proto = NULL;

// Forward declarations
static void s72_create_vtables(void);
static void s72_create_singletons(void);
static void s72_install_methods(void);
static void s72_repl(void);
static void s72_print_banner(void);

// Error handling
void s72_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    fprintf(stderr, "S72 Error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    
    va_end(args);
    exit(1);
}

void s72_type_error(const char *expected, S72Value got) {
    s72_error("Type error: expected %s, got %s", expected, "unknown");
}

// Value creation functions
S72Value s72_make_nil(void) {
    return S72_NIL;
}

S72Value s72_make_boolean(bool value) {
    return value ? S72_TRUE : S72_FALSE;
}

S72Value s72_make_number(double value) {
    return s72_number_new(value);
}

S72Value s72_make_string(const char *str) {
    return s72_string_new(str);
}

S72Value s72_make_symbol(const char *name) {
    return s72_symbol_new(name);
}

// Value testing functions
bool s72_is_nil(S72Value val) {
    return val.obj == S72_NIL.obj;
}

bool s72_is_boolean(S72Value val) {
    return val.obj == S72_TRUE.obj || val.obj == S72_FALSE.obj;
}

// Value extraction functions
bool s72_as_boolean(S72Value val) {
    return val.obj == S72_TRUE.obj;
}

double s72_as_number(S72Value val) {
    return s72_number_value(val);
}

const char *s72_as_string(S72Value val) {
    return s72_string_data(val);
}

const char *s72_as_symbol(S72Value val) {
    return s72_symbol_name(val);
}

// Message sending
S72Value s72_send(S72Value receiver, oop selector, int argc, S72Value *argv) {
    return s72_object_send(receiver, selector, argc, argv);
}

// Value printing
void s72_print_value(S72Value val) {
    if (s72_is_nil(val)) {
        printf("nil");
    } else if (s72_is_boolean(val)) {
        printf("%s", s72_as_boolean(val) ? "true" : "false");
    } else if (s72_is_number(val)) {
        // For M0, print numbers directly without message sending
        double num = s72_as_number(val);
        if (num == floor(num) && num >= -1e15 && num <= 1e15) {
            printf("%.0f", num);
        } else {
            printf("%.15g", num);
        }
    } else if (s72_is_string(val)) {
        printf("\"%s\"", s72_as_string(val));
    } else if (s72_is_symbol(val)) {
        printf("'%s", s72_as_symbol(val));
    } else {
        printf("<object>");
    }
}

char *s72_value_to_string(S72Value val) {
    static char buffer[256];
    
    if (s72_is_nil(val)) {
        strcpy(buffer, "nil");
    } else if (s72_is_boolean(val)) {
        strcpy(buffer, s72_as_boolean(val) ? "true" : "false");
    } else if (s72_is_number(val)) {
        double num = s72_as_number(val);
        if (num == floor(num) && num >= -1e15 && num <= 1e15) {
            snprintf(buffer, sizeof(buffer), "%.0f", num);
        } else {
            snprintf(buffer, sizeof(buffer), "%.15g", num);
        }
    } else if (s72_is_string(val)) {
        snprintf(buffer, sizeof(buffer), "\"%s\"", s72_as_string(val));
    } else if (s72_is_symbol(val)) {
        snprintf(buffer, sizeof(buffer), "'%s", s72_as_symbol(val));
    } else {
        strcpy(buffer, "<object>");
    }
    
    return buffer;
}

// Initialization
void s72_init(int *argc, char ***argv, char ***envp) {
    // Initialize libid
    struct __libid *libid = _libid_init(argc, argv, envp);
    if (!libid) {
        s72_error("Failed to initialize libid");
    }
    _libid = *libid;

    // Create vtables
    s72_create_vtables();

    // Intern selectors
    s72_symbol_intern_selectors();

    // Create singletons
    s72_create_singletons();

    // Initialize type systems
    s72_number_init();
    s72_string_init();
    s72_symbol_init();
    s72_object_init();
    s72_eval_init();

    // Install methods
    s72_install_methods();
}

void s72_shutdown(void) {
    // Cleanup (libid handles most of this)
    if (s72_global_env) {
        s72_env_free(s72_global_env);
        s72_global_env = NULL;
    }
}

// Create vtables for each type
static void s72_create_vtables(void) {
    // Create base object vtable
    s72_object_vtable = _libid_proto(_libid._object);
    if (!s72_object_vtable) {
        s72_error("Failed to create object vtable");
    }

    // Create type-specific vtables
    s72_nil_vtable = _libid_proto(s72_object_vtable);
    s72_boolean_vtable = _libid_proto(s72_object_vtable);
    s72_number_vtable = _libid_proto(s72_object_vtable);
    s72_string_vtable = _libid_proto(s72_object_vtable);
    s72_symbol_vtable = _libid_proto(s72_object_vtable);
    s72_list_vtable = _libid_proto(s72_object_vtable);
    s72_block_vtable = _libid_proto(s72_object_vtable);

    // Create prototype objects for allocation
    s72_number_proto = _libid_alloc(s72_number_vtable, 0);
    s72_string_proto = _libid_alloc(s72_string_vtable, 0);
    s72_symbol_proto = _libid_alloc(s72_symbol_vtable, 0);

    // Update vtables to match what libid actually creates
    s72_number_vtable = s72_number_proto->_vtable[-1];
    s72_string_vtable = s72_string_proto->_vtable[-1];
    s72_symbol_vtable = s72_symbol_proto->_vtable[-1];



    if (!s72_nil_vtable || !s72_boolean_vtable || !s72_number_vtable ||
        !s72_string_vtable || !s72_symbol_vtable || !s72_list_vtable ||
        !s72_block_vtable) {
        s72_error("Failed to create type vtables");
    }
}

// Create singleton objects
static void s72_create_singletons(void) {
    // Create nil
    S72_NIL.obj = NULL;  // nil is represented as NULL pointer

    // Create true and false
    S72_TRUE.obj = S72_ALLOC(s72_boolean_vtable, 0);
    S72_FALSE.obj = S72_ALLOC(s72_boolean_vtable, 0);

    if (!S72_TRUE.obj || !S72_FALSE.obj) {
        s72_error("Failed to create boolean singletons");
    }
}

// Install native methods
static void s72_install_methods(void) {
    // Methods are installed by each type's init function
    // This is called after all type init functions
}

// Print startup banner
static void s72_print_banner(void) {
    printf("S72 Interpreter v0.1 (M0 - Skeleton)\n");
    printf("Type expressions to evaluate, or 'quit' to exit.\n");
    printf("Examples: 42, \"hello\", 'symbol, (3 + 4)\n\n");
}

// REPL implementation
static void s72_repl(void) {
    char input[1024];
    
    s72_print_banner();
    
    while (true) {
        printf("s72> ");
        fflush(stdout);
        
        if (!fgets(input, sizeof(input), stdin)) {
            printf("\n");
            break;
        }
        
        // Remove trailing newline
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }
        
        // Check for quit command
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            break;
        }
        
        // Skip empty lines
        if (strlen(input) == 0) {
            continue;
        }
        
        // Parse and evaluate
        printf("DEBUG: About to parse input: %s\n", input);
        ASTNode *ast = s72_read_string(input);
        printf("DEBUG: Parse returned %p\n", ast);

        if (!ast) {
            printf("Parse error\n");
            continue;
        }

        printf("DEBUG: About to evaluate AST\n");
        S72Value result = s72_eval(ast, s72_global_env);
        printf("DEBUG: Evaluation returned %p\n", result.obj);

        // Print result
        s72_print_value(result);
        printf("\n");

        // Cleanup
        ast_free(ast);
    }
    
    printf("Goodbye!\n");
}

// Main function
int main(int argc, char **argv, char **envp) {
    // Initialize S72
    s72_init(&argc, &argv, &envp);
    
    // Run REPL
    s72_repl();
    
    // Shutdown
    s72_shutdown();
    
    return 0;
}
