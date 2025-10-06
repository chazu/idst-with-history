#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include "s72.h"
#include <stdio.h>

// Snapshot file format version
#define S72_SNAPSHOT_VERSION 1

// Snapshot file magic number
#define S72_SNAPSHOT_MAGIC 0x53373200  // "S72\0"

// Snapshot header structure
typedef struct {
    uint32_t magic;           // Magic number for file format validation
    uint32_t version;         // Snapshot format version
    uint32_t object_count;    // Number of objects in snapshot
    uint32_t root_object_id;  // ID of root object (global environment)
    uint64_t timestamp;       // Creation timestamp
    uint32_t checksum;        // Simple checksum for integrity
} S72SnapshotHeader;

// Object serialization types
typedef enum {
    S72_SERIAL_NIL,
    S72_SERIAL_BOOLEAN,
    S72_SERIAL_NUMBER,
    S72_SERIAL_STRING,
    S72_SERIAL_SYMBOL,
    S72_SERIAL_LIST,
    S72_SERIAL_BLOCK,
    S72_SERIAL_OBJECT,
    S72_SERIAL_ENVIRONMENT
} S72SerialType;

// Serialized object header
typedef struct {
    uint32_t object_id;       // Unique object ID in snapshot
    S72SerialType type;       // Object type
    uint32_t data_size;       // Size of object data following this header
    uint32_t ref_count;       // Number of references to other objects
} S72SerialObjectHeader;

// Object reference (for handling cycles and shared objects)
typedef struct {
    uint32_t target_id;       // ID of referenced object
    uint32_t offset;          // Offset within object data where reference occurs
} S72ObjectReference;

// Snapshot context for serialization/deserialization
typedef struct {
    FILE *file;               // File handle
    uint32_t next_object_id;  // Next available object ID
    void *object_map;         // Hash table: oop -> object_id (for serialization)
    void *id_map;             // Hash table: object_id -> oop (for deserialization)
    bool writing;             // True if writing, false if reading
} S72SnapshotContext;

// ============================================================================
// Public API
// ============================================================================

// Save the current object graph to a snapshot file
bool s72_snapshot_save(const char *filename);

// Load an object graph from a snapshot file
bool s72_snapshot_load(const char *filename);

// Save specific object and its dependencies
bool s72_snapshot_save_object(const char *filename, S72Value root_object);

// Load and return specific object from snapshot
S72Value s72_snapshot_load_object(const char *filename);

// ============================================================================
// Internal API (for implementation)
// ============================================================================

// Context management
S72SnapshotContext *s72_snapshot_context_create(FILE *file, bool writing);
void s72_snapshot_context_destroy(S72SnapshotContext *ctx);

// Object serialization
bool s72_serialize_object(S72SnapshotContext *ctx, S72Value obj);
S72Value s72_deserialize_object(S72SnapshotContext *ctx);

// Type-specific serialization
bool s72_serialize_nil(S72SnapshotContext *ctx, S72Value obj);
bool s72_serialize_boolean(S72SnapshotContext *ctx, S72Value obj);
bool s72_serialize_number(S72SnapshotContext *ctx, S72Value obj);
bool s72_serialize_string(S72SnapshotContext *ctx, S72Value obj);
bool s72_serialize_symbol(S72SnapshotContext *ctx, S72Value obj);
bool s72_serialize_list(S72SnapshotContext *ctx, S72Value obj);
bool s72_serialize_block(S72SnapshotContext *ctx, S72Value obj);
bool s72_serialize_object_obj(S72SnapshotContext *ctx, S72Value obj);
bool s72_serialize_environment(S72SnapshotContext *ctx, S72Env *env);

// Type-specific deserialization
S72Value s72_deserialize_nil(S72SnapshotContext *ctx);
S72Value s72_deserialize_boolean(S72SnapshotContext *ctx);
S72Value s72_deserialize_number(S72SnapshotContext *ctx);
S72Value s72_deserialize_string(S72SnapshotContext *ctx);
S72Value s72_deserialize_symbol(S72SnapshotContext *ctx);
S72Value s72_deserialize_list(S72SnapshotContext *ctx);
S72Value s72_deserialize_block(S72SnapshotContext *ctx);
S72Value s72_deserialize_object_obj(S72SnapshotContext *ctx);
S72Env *s72_deserialize_environment(S72SnapshotContext *ctx);

// Utility functions
uint32_t s72_calculate_checksum(const void *data, size_t size);
bool s72_write_header(S72SnapshotContext *ctx, const S72SnapshotHeader *header);
bool s72_read_header(S72SnapshotContext *ctx, S72SnapshotHeader *header);

// Object mapping for cycle detection
uint32_t s72_get_object_id(S72SnapshotContext *ctx, S72Value obj);
void s72_set_object_id(S72SnapshotContext *ctx, S72Value obj, uint32_t id);
S72Value s72_get_object_by_id(S72SnapshotContext *ctx, uint32_t id);
void s72_set_object_by_id(S72SnapshotContext *ctx, uint32_t id, S72Value obj);

#endif // SNAPSHOT_H
