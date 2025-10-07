#ifndef S72_ENV_H
#define S72_ENV_H

#include "s72.h"

// Forward declarations
typedef struct S72Value S72Value;

// Variable binding structure (improved from eval.h)
typedef struct S72Binding {
    char *name;
    S72Value value;
} S72Binding;

// Environment structure with dynamic array instead of linked list
typedef struct S72Env {
    struct S72Env *parent;          // Parent environment for scoping
    S72Binding *bindings;           // Dynamic array of bindings
    int binding_count;              // Number of active bindings
    int capacity;                   // Allocated capacity
} S72Env;

// Environment creation and destruction
S72Env *s72_env_new(S72Env *parent);
void s72_env_free(S72Env *env);

// Variable binding and lookup
void s72_env_bind(S72Env *env, const char *name, S72Value value);
S72Value s72_env_lookup(S72Env *env, const char *name);
bool s72_env_has_local_binding(S72Env *env, const char *name);
bool s72_env_has_binding(S72Env *env, const char *name);
bool s72_env_set(S72Env *env, const char *name, S72Value value);

// Environment hierarchy
S72Env *s72_env_create_child(S72Env *parent);

// Block parameter binding
void s72_env_bind_parameters(S72Env *env, int param_count, char **param_names, S72Value *args);

// Global environment management
void s72_env_init_global(void);
S72Env *s72_env_get_global(void);

// Global environment variable
extern S72Env *s72_global_env;

// Debug utilities
void s72_env_debug_print(S72Env *env, int depth);

#endif // S72_ENV_H
