#include "snapshot.h"
#include "eval.h"
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
    ctx->id_map = hash_table_create();
    ctx->writing = writing;
    
    if (!ctx->object_map || !ctx->id_map) {
        s72_snapshot_context_destroy(ctx);
        return NULL;
    }
    
    return ctx;
}

void s72_snapshot_context_destroy(S72SnapshotContext *ctx) {
    if (!ctx) return;
    
    if (ctx->object_map) hash_table_destroy((HashTable *)ctx->object_map);
    if (ctx->id_map) hash_table_destroy((HashTable *)ctx->id_map);
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

S72Value s72_get_object_by_id(S72SnapshotContext *ctx, uint32_t id) {
    // For simplicity, we'll implement this as a linear search for now
    // In a real implementation, we'd use a proper reverse mapping
    S72Value nil_val = s72_make_nil();
    return nil_val; // TODO: Implement proper reverse lookup
}

void s72_set_object_by_id(S72SnapshotContext *ctx, uint32_t id, S72Value obj) {
    // TODO: Implement proper reverse mapping
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
    
    // Serialize global environment (simplified for M4)
    printf("Saving snapshot to %s...\n", filename);
    printf("Note: Full serialization not yet implemented - saving placeholder\n");
    
    // Update header with actual counts
    fseek(file, 0, SEEK_SET);
    header.object_count = 1; // Just the global environment for now
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
    printf("Note: Full deserialization not yet implemented - loading placeholder\n");
    
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
