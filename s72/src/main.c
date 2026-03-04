#include "s72.h"
#include "reader.h"
#include "eval.h"
#include "object.h"
#include "snapshot.h"
#include "types/number.h"
#include "types/string.h"
#include "types/symbol.h"
#include "types/boolean.h"
#include "types/block.h"
#include "types/list.h"
#include "types/array.h"
#include "types/transcript.h"
#include "types/turtle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

// Global libid instance
struct __libid *_libid;

// Global selectors
oop SEL_PLUS = NULL;
oop SEL_MINUS = NULL;
oop SEL_MULTIPLY = NULL;
oop SEL_DIVIDE = NULL;
oop SEL_EQUALS = NULL;
oop SEL_LESS_THAN = NULL;
oop SEL_GREATER_THAN = NULL;
oop SEL_LESS_EQUAL = NULL;
oop SEL_GREATER_EQUAL = NULL;
oop SEL_PRINT = NULL;
oop SEL_VALUE = NULL;
oop SEL_TO = NULL;
oop SEL_BECOME = NULL;
oop SEL_DOES_NOT_UNDERSTAND = NULL;

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
oop s72_array_vtable = NULL;
oop s72_block_vtable = NULL;
oop s72_transcript_vtable = NULL;
oop s72_turtle_vtable = NULL;
oop s72_canvas_vtable = NULL;
oop s72_object_vtable = NULL;

// Global prototype objects - no longer needed, using vtables directly

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

S72Value s72_make_list_empty(void) {
    return s72_list_empty();
}

S72Value s72_make_list_cons(S72Value first, S72Value rest) {
    return s72_list_cons(first, rest);
}

S72Value s72_make_transcript(void) {
    return s72_transcript_singleton();
}

// Value testing functions
bool s72_is_nil(S72Value val) {
    return val.obj == S72_NIL.obj || val.obj == NULL;
}

// s72_is_boolean is now defined in types/boolean.c

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
    } else if (s72_is_string(val)) {
        printf("\"%s\"", s72_as_string(val));
    } else if (s72_is_number(val)) {
        // For M0, print numbers directly without message sending
        double num = s72_as_number(val);
        if (num == floor(num) && num >= -1e15 && num <= 1e15) {
            printf("%.0f", num);
        } else {
            printf("%.15g", num);
        }
    } else if (s72_is_list(val)) {
        // Check list first before symbol (both may return true due to vtable inheritance)
        printf("<list>");
    } else if (s72_is_symbol(val)) {
        printf("'%s", s72_as_symbol(val));
    } else if (s72_is_block(val)) {
        // Call the block's print method
        oop print_sel = S72_INTERN("print");
        s72_object_send(val, print_sel, 0, NULL);
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
    _libid = _libid_init(argc, argv, envp);
    if (!_libid) {
        s72_error("Failed to initialize libid");
    }

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
    s72_boolean_init();
    s72_block_init();
    s72_list_init();
    s72_array_init();
    s72_transcript_init();
    s72_turtle_init();
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
    s72_object_vtable = S72_PROTO(_libid->_object);
    if (!s72_object_vtable) {
        s72_error("Failed to create object vtable");
    }

    // Create type-specific vtables
    s72_nil_vtable = S72_PROTO(s72_object_vtable);
    s72_boolean_vtable = S72_PROTO(s72_object_vtable);
    s72_number_vtable = S72_PROTO(s72_object_vtable);
    s72_string_vtable = S72_PROTO(s72_object_vtable);
    s72_symbol_vtable = S72_PROTO(s72_object_vtable);
    s72_list_vtable = S72_PROTO(s72_object_vtable);
    s72_array_vtable = S72_PROTO(s72_object_vtable);
    s72_block_vtable = S72_PROTO(s72_object_vtable);
    s72_transcript_vtable = S72_PROTO(s72_object_vtable);
    s72_turtle_vtable = S72_PROTO(s72_object_vtable);
    s72_canvas_vtable = S72_PROTO(s72_object_vtable);

    // No need for prototype objects - we allocate directly with vtables

    if (!s72_nil_vtable || !s72_boolean_vtable || !s72_number_vtable ||
        !s72_string_vtable || !s72_symbol_vtable || !s72_list_vtable ||
        !s72_array_vtable ||
        !s72_block_vtable || !s72_transcript_vtable || !s72_turtle_vtable ||
        !s72_canvas_vtable) {
        s72_error("Failed to create type vtables");
    }
}

// Create singleton objects
static void s72_create_singletons(void) {
    // Create nil as an actual object with nil vtable
    S72_NIL.obj = S72_ALLOC(s72_nil_vtable, 0);

    // Create true and false
    S72_TRUE.obj = S72_ALLOC(s72_boolean_vtable, 0);
    S72_FALSE.obj = S72_ALLOC(s72_boolean_vtable, 0);

    if (!S72_NIL.obj || !S72_TRUE.obj || !S72_FALSE.obj) {
        s72_error("Failed to create singleton objects");
    }
}

// Install native methods
static void s72_install_methods(void) {
    // Methods are installed by each type's init function
    // This is called after all type init functions
}

// Load and evaluate startup file
static bool s72_load_startup_file(const char *filename) {
    printf("Loading startup file: %s\n", filename);

    // Check if file exists
    FILE *test_file = fopen(filename, "r");
    if (!test_file) {
        printf("Warning: Startup file not found: %s\n", filename);
        return false;
    }
    fclose(test_file);

    // Read and parse the file
    ASTNode *ast = s72_read_file(filename);
    if (!ast) {
        printf("Error: Failed to parse startup file: %s\n", filename);
        return false;
    }

    // Evaluate the startup file
    S72Value result = s72_eval(ast, s72_global_env);

    // Check if evaluation was successful
    if (s72_is_nil(result)) {
        printf("Warning: Startup file evaluation returned nil\n");
    } else {
        printf("Startup file loaded successfully\n");
    }

    // Cleanup
    ast_free(ast);
    return true;
}

// Print startup banner
static void s72_print_banner(void) {
    printf("S72 Interpreter v0.1 (M4 - Image Persistence & Standard Library)\n");
    printf("Type expressions to evaluate, or 'quit' to exit.\n");
    printf("Examples: 42, \"hello\", 'symbol, (3 + 4)\n");
    printf("Commands: :save <file>, :load <file>, :help\n\n");
}

// Print help message
static void s72_print_help(void) {
    printf("S72 Interpreter Commands:\n");
    printf("  :help          - Show this help message\n");
    printf("  :save <file>   - Save current session to image file\n");
    printf("  :load <file>   - Load session from image file\n");
    printf("  :quit, :exit   - Exit the interpreter\n");
    printf("  quit, exit     - Exit the interpreter\n");
    printf("\nExamples:\n");
    printf("  42             - Evaluate number literal\n");
    printf("  \"hello\"        - Evaluate string literal\n");
    printf("  'symbol        - Evaluate symbol literal\n");
    printf("  (3 + 4)        - Evaluate arithmetic expression\n");
    printf("  (show: \"hi\")    - Use standard library function\n");
    printf("\n");
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
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0 ||
            strcmp(input, ":quit") == 0 || strcmp(input, ":exit") == 0) {
            break;
        }

        // Check for help command
        if (strcmp(input, ":help") == 0 || strcmp(input, "help") == 0) {
            s72_print_help();
            continue;
        }

        // Check for save command
        if (strncmp(input, ":save ", 6) == 0) {
            const char *filename = input + 6;
            while (*filename == ' ') filename++; // Skip spaces
            if (*filename) {
                if (s72_snapshot_save(filename)) {
                    printf("Session saved to %s\n", filename);
                } else {
                    printf("Failed to save session to %s\n", filename);
                }
            } else {
                printf("Usage: :save <filename>\n");
            }
            continue;
        }

        // Check for load command
        if (strncmp(input, ":load ", 6) == 0) {
            const char *filename = input + 6;
            while (*filename == ' ') filename++; // Skip spaces
            if (*filename) {
                if (s72_snapshot_load(filename)) {
                    printf("Session loaded from %s\n", filename);
                } else {
                    printf("Failed to load session from %s\n", filename);
                }
            } else {
                printf("Usage: :load <filename>\n");
            }
            continue;
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
            // Check if input is empty or contains only whitespace/comments
            bool is_empty = true;
            for (int i = 0; input[i]; i++) {
                if (input[i] != ' ' && input[i] != '\t' && input[i] != '\n' && input[i] != '\r') {
                    if (input[i] == ';') {
                        // Rest of line is comment, so line is effectively empty
                        break;
                    }
                    is_empty = false;
                    break;
                }
            }

            if (is_empty) {
                // Empty line or comment-only line, just continue
                continue;
            } else {
                printf("Parse error\n");
                continue;
            }
        }

        printf("DEBUG: AST type = %d\n", ast->type);
        if (ast->type == AST_LIST) {
            printf("DEBUG: List has %d elements\n", ast->data.list.count);
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

// Print usage information
static void s72_print_usage(const char *program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("Options:\n");
    printf("  -h, --help           Show this help message\n");
    printf("  -l, --load <file>    Load image file at startup\n");
    printf("  -s, --save <file>    Save image file at exit\n");
    printf("  --no-init            Skip loading lang/init.s72\n");
    printf("  --version            Show version information\n");
    printf("\n");
}

// Main function
int main(int argc, char **argv, char **envp) {
    const char *load_file = NULL;
    const char *save_file = NULL;
    bool load_init = true;

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            s72_print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("S72 Interpreter v0.1 (M4 - Image Persistence & Standard Library)\n");
            return 0;
        } else if (strcmp(argv[i], "--no-init") == 0) {
            load_init = false;
        } else if ((strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--load") == 0) && i + 1 < argc) {
            load_file = argv[++i];
        } else if ((strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--save") == 0) && i + 1 < argc) {
            save_file = argv[++i];
        } else {
            printf("Unknown option: %s\n", argv[i]);
            s72_print_usage(argv[0]);
            return 1;
        }
    }

    // Initialize S72
    s72_init(&argc, &argv, &envp);

    // Load image file if specified
    if (load_file) {
        printf("Loading image file: %s\n", load_file);
        if (!s72_snapshot_load(load_file)) {
            printf("Warning: Failed to load image file\n");
        }
    }

    // Load startup file (M4 feature) unless disabled
    if (load_init) {
        s72_load_startup_file("lang/init.s72");
    }

    // Run REPL
    s72_repl();

    // Save image file if specified
    if (save_file) {
        printf("Saving image file: %s\n", save_file);
        if (!s72_snapshot_save(save_file)) {
            printf("Warning: Failed to save image file\n");
        }
    }

    // Shutdown
    s72_shutdown();

    return 0;
}
