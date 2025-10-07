#include "snapshot.h"
#include "eval.h"
#include "env.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Simple hash table implementation for object mapping
#define HASH_TABLE_SIZE 1024

typedef struct HashEntry {
    void *key;
    uint32_t value;
    struct HashEntry *next;
} HashEntry;

typedef struct {
    HashEntry *buckets[HASH_TABLE_SIZE];
} HashTable;

// Hash function for pointers
static uint32_t hash_pointer(void *ptr) {
    uintptr_t addr = (uintptr_t)ptr;
    return (uint32_t)(addr % HASH_TABLE_SIZE);
}

// Hash table operations
static HashTable *hash_table_create(void) {
    HashTable *table = calloc(1, sizeof(HashTable));
    return table;
}

static void hash_table_destroy(HashTable *table) {
    if (!table) return;
    
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashEntry *entry = table->buckets[i];
        while (entry) {
            HashEntry *next = entry->next;
            free(entry);
            entry = next;
        }
    }
    free(table);
}

static void hash_table_set(HashTable *table, void *key, uint32_t value) {
    uint32_t hash = hash_pointer(key);
    HashEntry *entry = table->buckets[hash];
    
    // Check if key already exists
    while (entry) {
        if (entry->key == key) {
            entry->value = value;
            return;
        }
        entry = entry->next;
    }
    
    // Create new entry
    entry = malloc(sizeof(HashEntry));
    entry->key = key;
    entry->value = value;
    entry->next = table->buckets[hash];
    table->buckets[hash] = entry;
}

static uint32_t hash_table_get(HashTable *table, void *key) {
    uint32_t hash = hash_pointer(key);
    HashEntry *entry = table->buckets[hash];
    
    while (entry) {
        if (entry->key == key) {
            return entry->value;
        }
        entry = entry->next;
    }
    
    return 0; // Not found
}

// ============================================================================
// Context Management
// ============================================================================

S72SnapshotContext *s72_snapshot_context_create(FILE *file, bool writing) {
    S72SnapshotContext *ctx = malloc(sizeof(S72SnapshotContext));
    if (!ctx) return NULL;
    
    ctx->file = file;
    ctx->next_object_id = 1; // Start from 1, 0 means "not found"
    ctx->object_map = hash_table_create();
    ctx->reverse_map = NULL; // Created on demand
    ctx->writing = writing;

    if (!ctx->object_map) {
        s72_snapshot_context_destroy(ctx);
        return NULL;
    }
    
    return ctx;
}

// Forward declaration
static void s72_reverse_map_destroy(void *map);

void s72_snapshot_context_destroy(S72SnapshotContext *ctx) {
    if (!ctx) return;

    if (ctx->object_map) hash_table_destroy((HashTable *)ctx->object_map);
    if (ctx->reverse_map) s72_reverse_map_destroy(ctx->reverse_map);
    free(ctx);
}

// ============================================================================
// Object ID Management
// ============================================================================

uint32_t s72_get_object_id(S72SnapshotContext *ctx, S72Value obj) {
    if (!ctx || !ctx->object_map) return 0;
    return hash_table_get((HashTable *)ctx->object_map, obj.obj);
}

void s72_set_object_id(S72SnapshotContext *ctx, S72Value obj, uint32_t id) {
    if (!ctx || !ctx->object_map) return;
    hash_table_set((HashTable *)ctx->object_map, obj.obj, id);
}

// Reverse mapping structure for ID -> Object lookup
typedef struct {
    uint32_t *ids;
    S72Value *objects;
    int count;
    int capacity;
} S72ReverseMap;

static S72ReverseMap *s72_reverse_map_create(void) {
    S72ReverseMap *map = malloc(sizeof(S72ReverseMap));
    if (!map) return NULL;

    map->ids = NULL;
    map->objects = NULL;
    map->count = 0;
    map->capacity = 0;
    return map;
}

static void s72_reverse_map_destroy(void *map_ptr) {
    S72ReverseMap *map = (S72ReverseMap *)map_ptr;
    if (!map) return;
    free(map->ids);
    free(map->objects);
    free(map);
}

static void s72_reverse_map_add(S72ReverseMap *map, uint32_t id, S72Value obj) {
    if (!map) return;

    // Resize if needed
    if (map->count >= map->capacity) {
        int new_capacity = map->capacity == 0 ? 16 : map->capacity * 2;
        uint32_t *new_ids = realloc(map->ids, new_capacity * sizeof(uint32_t));
        S72Value *new_objects = realloc(map->objects, new_capacity * sizeof(S72Value));

        if (!new_ids || !new_objects) {
            free(new_ids);
            free(new_objects);
            return;
        }

        map->ids = new_ids;
        map->objects = new_objects;
        map->capacity = new_capacity;
    }

    map->ids[map->count] = id;
    map->objects[map->count] = obj;
    map->count++;
}

static S72Value s72_reverse_map_get(S72ReverseMap *map, uint32_t id) {
    if (!map) return S72_NIL;

    for (int i = 0; i < map->count; i++) {
        if (map->ids[i] == id) {
            return map->objects[i];
        }
    }
    return S72_NIL;
}

S72Value s72_get_object_by_id(S72SnapshotContext *ctx, uint32_t id) {
    if (!ctx || !ctx->reverse_map) return S72_NIL;
    return s72_reverse_map_get((S72ReverseMap *)ctx->reverse_map, id);
}

void s72_set_object_by_id(S72SnapshotContext *ctx, uint32_t id, S72Value obj) {
    if (!ctx) return;

    // Create reverse map if it doesn't exist
    if (!ctx->reverse_map) {
        ctx->reverse_map = s72_reverse_map_create();
    }

    if (ctx->reverse_map) {
        s72_reverse_map_add((S72ReverseMap *)ctx->reverse_map, id, obj);
    }
}

// ============================================================================
// Utility Functions
// ============================================================================

uint32_t s72_calculate_checksum(const void *data, size_t size) {
    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t checksum = 0;
    
    for (size_t i = 0; i < size; i++) {
        checksum = (checksum << 1) ^ bytes[i];
    }
    
    return checksum;
}

bool s72_write_header(S72SnapshotContext *ctx, const S72SnapshotHeader *header) {
    return fwrite(header, sizeof(S72SnapshotHeader), 1, ctx->file) == 1;
}

bool s72_read_header(S72SnapshotContext *ctx, S72SnapshotHeader *header) {
    return fread(header, sizeof(S72SnapshotHeader), 1, ctx->file) == 1;
}

// ============================================================================
// High-Level API
// ============================================================================

bool s72_snapshot_save(const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        s72_error("Cannot create snapshot file: %s", filename);
        return false;
    }
    
    S72SnapshotContext *ctx = s72_snapshot_context_create(file, true);
    if (!ctx) {
        fclose(file);
        return false;
    }
    
    // Create header
    S72SnapshotHeader header = {
        .magic = S72_SNAPSHOT_MAGIC,
        .version = S72_SNAPSHOT_VERSION,
        .object_count = 0, // Will be updated later
        .root_object_id = 1, // Global environment will be object 1
        .timestamp = (uint64_t)time(NULL),
        .checksum = 0 // Will be calculated later
    };
    
    // Write placeholder header
    if (!s72_write_header(ctx, &header)) {
        s72_snapshot_context_destroy(ctx);
        fclose(file);
        return false;
    }
    
    // Serialize global environment
    printf("Saving snapshot to %s...\n", filename);

    S72Env *global_env = s72_env_get_global();
    if (global_env) {
        // Serialize the global environment bindings
        uint32_t binding_count = (uint32_t)global_env->binding_count;

        // Write binding count
        if (fwrite(&binding_count, sizeof(uint32_t), 1, file) != 1) {
            printf("Error: Failed to write binding count\n");
            s72_snapshot_context_destroy(ctx);
            fclose(file);
            return false;
        }

        // Write each binding
        for (int i = 0; i < global_env->binding_count; i++) {
            S72Binding *binding = &global_env->bindings[i];

            // Write name length and name
            uint32_t name_len = (uint32_t)strlen(binding->name);
            if (fwrite(&name_len, sizeof(uint32_t), 1, file) != 1 ||
                fwrite(binding->name, 1, name_len, file) != name_len) {
                printf("Error: Failed to write binding name\n");
                s72_snapshot_context_destroy(ctx);
                fclose(file);
                return false;
            }

            // For now, we'll serialize the object pointer as a simple value
            // In a full implementation, we'd recursively serialize the object
            if (fwrite(&binding->value.obj, sizeof(oop), 1, file) != 1) {
                printf("Error: Failed to write binding value\n");
                s72_snapshot_context_destroy(ctx);
                fclose(file);
                return false;
            }
        }

        printf("Serialized %d global bindings\n", binding_count);
    }

    // Update header with actual counts
    fseek(file, 0, SEEK_SET);
    header.object_count = global_env ? global_env->binding_count : 0;
    s72_write_header(ctx, &header);
    
    s72_snapshot_context_destroy(ctx);
    fclose(file);
    
    printf("Snapshot saved successfully\n");
    return true;
}

bool s72_snapshot_load(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        s72_error("Cannot open snapshot file: %s", filename);
        return false;
    }
    
    S72SnapshotContext *ctx = s72_snapshot_context_create(file, false);
    if (!ctx) {
        fclose(file);
        return false;
    }
    
    // Read header
    S72SnapshotHeader header;
    if (!s72_read_header(ctx, &header)) {
        s72_error("Cannot read snapshot header");
        s72_snapshot_context_destroy(ctx);
        fclose(file);
        return false;
    }
    
    // Validate header
    if (header.magic != S72_SNAPSHOT_MAGIC) {
        s72_error("Invalid snapshot file format");
        s72_snapshot_context_destroy(ctx);
        fclose(file);
        return false;
    }
    
    if (header.version != S72_SNAPSHOT_VERSION) {
        s72_error("Unsupported snapshot version: %u", header.version);
        s72_snapshot_context_destroy(ctx);
        fclose(file);
        return false;
    }
    
    printf("Loading snapshot from %s...\n", filename);

    // Read binding count
    uint32_t binding_count;
    if (fread(&binding_count, sizeof(uint32_t), 1, file) != 1) {
        printf("Error: Failed to read binding count\n");
        s72_snapshot_context_destroy(ctx);
        fclose(file);
        return false;
    }

    printf("Loading %d global bindings\n", binding_count);

    // Get global environment
    S72Env *global_env = s72_env_get_global();
    if (!global_env) {
        printf("Error: Global environment not initialized\n");
        s72_snapshot_context_destroy(ctx);
        fclose(file);
        return false;
    }

    // Read each binding
    for (uint32_t i = 0; i < binding_count; i++) {
        // Read name length and name
        uint32_t name_len;
        if (fread(&name_len, sizeof(uint32_t), 1, file) != 1) {
            printf("Error: Failed to read binding name length\n");
            s72_snapshot_context_destroy(ctx);
            fclose(file);
            return false;
        }

        char *name = malloc(name_len + 1);
        if (!name || fread(name, 1, name_len, file) != name_len) {
            printf("Error: Failed to read binding name\n");
            free(name);
            s72_snapshot_context_destroy(ctx);
            fclose(file);
            return false;
        }
        name[name_len] = '\0';

        // Read value (simplified - just the object pointer)
        oop obj_ptr;
        if (fread(&obj_ptr, sizeof(oop), 1, file) != 1) {
            printf("Error: Failed to read binding value\n");
            free(name);
            s72_snapshot_context_destroy(ctx);
            fclose(file);
            return false;
        }

        // Create S72Value and bind it
        S72Value value = {obj_ptr};
        s72_env_bind(global_env, name, value);

        printf("Restored binding: %s -> %p\n", name, obj_ptr);
        free(name);
    }

    s72_snapshot_context_destroy(ctx);
    fclose(file);

    printf("Snapshot loaded successfully\n");
    return true;
}

// Simplified API for M4
bool s72_snapshot_save_object(const char *filename, S72Value root_object) {
    return s72_snapshot_save(filename);
}

S72Value s72_snapshot_load_object(const char *filename) {
    if (s72_snapshot_load(filename)) {
        return s72_make_boolean(true);
    } else {
        return s72_make_nil();
    }
}
