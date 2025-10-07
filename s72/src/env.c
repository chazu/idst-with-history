#include "env.h"
#include "types/transcript.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global environment
S72Env *s72_global_env = NULL;

// Environment creation and destruction
S72Env *s72_env_new(S72Env *parent) {
    S72Env *env = malloc(sizeof(S72Env));
    if (!env) {
        s72_error("Failed to allocate environment");
        return NULL;
    }

    env->parent = parent;
    env->bindings = NULL;
    env->binding_count = 0;
    env->capacity = 0;
    
    return env;
}

void s72_env_free(S72Env *env) {
    if (!env) return;

    // Free all bindings
    for (int i = 0; i < env->binding_count; i++) {
        free(env->bindings[i].name);
    }
    free(env->bindings);
    free(env);
}

// Resize bindings array if needed
static void s72_env_ensure_capacity(S72Env *env, int min_capacity) {
    if (env->capacity >= min_capacity) return;
    
    int new_capacity = env->capacity == 0 ? 8 : env->capacity * 2;
    while (new_capacity < min_capacity) {
        new_capacity *= 2;
    }
    
    S72Binding *new_bindings = realloc(env->bindings, new_capacity * sizeof(S72Binding));
    if (!new_bindings) {
        s72_error("Failed to resize environment bindings");
        return;
    }
    
    env->bindings = new_bindings;
    env->capacity = new_capacity;
}

// Variable binding
void s72_env_bind(S72Env *env, const char *name, S72Value value) {
    if (!env || !name) return;

    printf("DEBUG: s72_env_bind - binding '%s' to %p in env %p\n", name, value.obj, env);

    // Check if binding already exists and update it
    for (int i = 0; i < env->binding_count; i++) {
        if (strcmp(env->bindings[i].name, name) == 0) {
            env->bindings[i].value = value;
            return;
        }
    }

    // Add new binding
    s72_env_ensure_capacity(env, env->binding_count + 1);
    
    int index = env->binding_count;
    env->bindings[index].name = strdup(name);
    if (!env->bindings[index].name) {
        s72_error("Failed to copy binding name");
        return;
    }
    
    env->bindings[index].value = value;
    env->binding_count++;
}

// Variable lookup with environment chain traversal
S72Value s72_env_lookup(S72Env *env, const char *name) {
    if (!env || !name) return S72_NIL;

    printf("DEBUG: s72_env_lookup - looking up '%s' in env %p\n", name, env);

    // Search current environment
    for (int i = 0; i < env->binding_count; i++) {
        if (strcmp(env->bindings[i].name, name) == 0) {
            printf("DEBUG: Found binding for '%s': %p\n", name, env->bindings[i].value.obj);
            return env->bindings[i].value;
        }
    }

    // Search parent environment
    if (env->parent) {
        return s72_env_lookup(env->parent, name);
    }

    printf("DEBUG: No binding found for '%s'\n", name);
    return S72_NIL;
}

// Check if environment has a binding (local only)
bool s72_env_has_local_binding(S72Env *env, const char *name) {
    if (!env || !name) return false;
    
    for (int i = 0; i < env->binding_count; i++) {
        if (strcmp(env->bindings[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

// Check if environment chain has a binding
bool s72_env_has_binding(S72Env *env, const char *name) {
    S72Value result = s72_env_lookup(env, name);
    return !s72_is_nil(result);
}

// Set variable in existing binding (traverses environment chain)
bool s72_env_set(S72Env *env, const char *name, S72Value value) {
    if (!env || !name) return false;

    // Search current environment
    for (int i = 0; i < env->binding_count; i++) {
        if (strcmp(env->bindings[i].name, name) == 0) {
            env->bindings[i].value = value;
            return true;
        }
    }

    // Search parent environment
    if (env->parent) {
        return s72_env_set(env->parent, name, value);
    }

    return false; // Variable not found
}

// Create child environment for block execution
S72Env *s72_env_create_child(S72Env *parent) {
    return s72_env_new(parent);
}

// Bind multiple parameters at once (for block parameter binding)
void s72_env_bind_parameters(S72Env *env, int param_count, char **param_names, S72Value *args) {
    if (!env || param_count <= 0) return;
    
    int bind_count = param_count;
    for (int i = 0; i < bind_count; i++) {
        if (param_names && param_names[i] && args) {
            printf("DEBUG: Binding parameter '%s' to value %p\n", 
                   param_names[i], args[i].obj);
            s72_env_bind(env, param_names[i], args[i]);
        }
    }
}

// Global environment initialization
void s72_env_init_global(void) {
    if (s72_global_env) {
        s72_env_free(s72_global_env);
    }
    
    s72_global_env = s72_env_new(NULL);

    // Bind global singletons to the environment
    s72_env_bind(s72_global_env, "nil", S72_NIL);
    s72_env_bind(s72_global_env, "true", S72_TRUE);
    s72_env_bind(s72_global_env, "false", S72_FALSE);
    s72_env_bind(s72_global_env, "Transcript", s72_transcript_singleton());

    // Bind Turtle singleton when it's available
    extern oop s72_turtle_singleton;
    if (s72_turtle_singleton) {
        S72Value turtle_value = { .obj = s72_turtle_singleton };
        s72_env_bind(s72_global_env, "Turtle", turtle_value);
    }
}

// Get global environment
S72Env *s72_env_get_global(void) {
    return s72_global_env;
}

// Debug: Print environment contents
void s72_env_debug_print(S72Env *env, int depth) {
    if (!env) return;
    
    for (int i = 0; i < depth; i++) printf("  ");
    printf("Environment %p (parent: %p):\n", env, env->parent);
    
    for (int i = 0; i < env->binding_count; i++) {
        for (int j = 0; j < depth + 1; j++) printf("  ");
        printf("'%s' -> %p\n", env->bindings[i].name, env->bindings[i].value.obj);
    }
    
    if (env->parent) {
        s72_env_debug_print(env->parent, depth + 1);
    }
}
