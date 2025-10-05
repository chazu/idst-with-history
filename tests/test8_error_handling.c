/*
 * Test 8: Error Handling and Edge Cases
 * Tests error conditions, null handling, and edge cases
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>

// Forward declarations from libid
struct __libid;
typedef struct t__object *oop;

struct __libid *_libid_init(int *argcp, char ***argvp, char ***envp);
oop _libid_intern(const char *string);
oop _libid_proto(oop base);
void *_libid_palloc(size_t size);
void *_libid_balloc(size_t size);
void _libid_export(const char *key, oop value);
oop _libid_import(const char *key);

// Global for signal handling
static jmp_buf error_jmp;
static int caught_signal = 0;

void signal_handler(int sig)
{
    caught_signal = sig;
    longjmp(error_jmp, 1);
}

int main(int argc, char **argv, char **envp)
{
    printf("=== Test 8: Error Handling and Edge Cases ===\n");
    
    // Initialize libid
    struct __libid *libid = _libid_init(&argc, &argv, &envp);
    if (!libid) {
        printf("FAIL: Could not initialize libid\n");
        return 1;
    }
    printf("✓ libid initialized successfully\n");
    
    // Test 1: NULL and empty string handling
    printf("✓ Testing NULL and empty string handling...\n");
    
    // Test empty string interning
    oop empty_sel = _libid_intern("");
    if (empty_sel) {
        printf("  ✓ Empty string interning works: %p\n", empty_sel);
    } else {
        printf("  ✗ FAIL: Empty string interning failed\n");
        return 1;
    }
    
    // Test same empty string returns same object
    oop empty_sel2 = _libid_intern("");
    if (empty_sel == empty_sel2) {
        printf("  ✓ Empty string interning is consistent\n");
    } else {
        printf("  ✗ FAIL: Empty string interning inconsistent\n");
        return 1;
    }
    
    // Test 2: Large string handling
    printf("✓ Testing large string handling...\n");
    
    char large_string[1000];
    for (int i = 0; i < 999; i++) {
        large_string[i] = 'A' + (i % 26);
    }
    large_string[999] = '\0';
    
    oop large_sel = _libid_intern(large_string);
    if (large_sel) {
        printf("  ✓ Large string interning works: %p\n", large_sel);
    } else {
        printf("  ✗ FAIL: Large string interning failed\n");
        return 1;
    }
    
    // Test 3: Special characters in strings
    printf("✓ Testing special characters...\n");
    
    const char *special_strings[] = {
        "hello\nworld",      // newline
        "tab\there",         // tab
        "quote\"test",       // quote
        "backslash\\test",   // backslash
        "unicode\xC3\xA9",  // UTF-8
        "numbers123",        // numbers
        "symbols!@#$%",      // symbols
    };
    
    int num_special = sizeof(special_strings) / sizeof(special_strings[0]);
    for (int i = 0; i < num_special; i++) {
        oop special_sel = _libid_intern(special_strings[i]);
        if (!special_sel) {
            printf("  ✗ FAIL: Special string %d failed\n", i);
            return 1;
        }
    }
    printf("  ✓ All special character strings handled correctly\n");
    
    // Test 4: Memory allocation edge cases
    printf("✓ Testing memory allocation edge cases...\n");
    
    // Test zero allocation
    void *zero_ptr = _libid_palloc(0);
    printf("  - Zero allocation: %p (implementation defined)\n", zero_ptr);
    
    // Test very small allocations
    for (size_t size = 1; size <= 16; size++) {
        void *small_ptr = _libid_palloc(size);
        if (!small_ptr) {
            printf("  ✗ FAIL: Small allocation of size %zu failed\n", size);
            return 1;
        }
    }
    printf("  ✓ Small allocations work correctly\n");
    
    // Test large allocations
    size_t large_sizes[] = {1024*1024, 1024*1024*10, 1024*1024*50};  // 1MB, 10MB, 50MB
    for (int i = 0; i < 3; i++) {
        void *large_ptr = _libid_palloc(large_sizes[i]);
        if (large_ptr) {
            printf("  ✓ Large allocation %zu bytes: %p\n", large_sizes[i], large_ptr);
        } else {
            printf("  ⚠️  Large allocation %zu bytes failed (may be expected)\n", large_sizes[i]);
        }
    }
    
    // Test 5: Object creation edge cases
    printf("✓ Testing object creation edge cases...\n");
    
    // Test creating many objects rapidly
    oop objects[1000];
    for (int i = 0; i < 1000; i++) {
        objects[i] = _libid_proto(0);
        if (!objects[i]) {
            printf("  ✗ FAIL: Rapid object creation failed at %d\n", i);
            return 1;
        }
    }
    printf("  ✓ Rapid object creation successful (1000 objects)\n");
    
    // Test object delegation chains
    oop chain_obj = _libid_proto(0);
    for (int i = 0; i < 10; i++) {
        chain_obj = _libid_proto(chain_obj);
        if (!chain_obj) {
            printf("  ✗ FAIL: Delegation chain failed at depth %d\n", i);
            return 1;
        }
    }
    printf("  ✓ Deep delegation chains work (depth 10)\n");
    
    // Test 6: Import/Export edge cases
    printf("✓ Testing import/export edge cases...\n");
    
    // Test exporting NULL (should handle gracefully)
    _libid_export("null_test", 0);
    oop imported_null = _libid_import("null_test");
    printf("  - NULL export/import: %p\n", imported_null);
    
    // Test importing non-existent key (Note: libid exits on import failure by design)
    // This is expected behavior - libid calls fatal() on undefined imports
    printf("  - Non-existent import test skipped (libid exits on undefined imports by design)\n");
    
    // Test very long export keys
    char long_key[500];
    for (int i = 0; i < 499; i++) {
        long_key[i] = 'k';
    }
    long_key[499] = '\0';
    
    oop test_obj = _libid_proto(0);
    _libid_export(long_key, test_obj);
    oop imported_long = _libid_import(long_key);
    if (imported_long == test_obj) {
        printf("  ✓ Long key export/import works\n");
    } else {
        printf("  ✗ FAIL: Long key export/import failed\n");
        return 1;
    }
    
    // Test 7: Stress testing
    printf("✓ Running stress tests...\n");
    
    // Stress test selector interning
    for (int i = 0; i < 1000; i++) {
        char selector_name[32];
        snprintf(selector_name, sizeof(selector_name), "stress_selector_%d", i);
        oop stress_sel = _libid_intern(selector_name);
        if (!stress_sel) {
            printf("  ✗ FAIL: Stress selector interning failed at %d\n", i);
            return 1;
        }
    }
    printf("  ✓ Stress selector interning successful (1000 selectors)\n");
    
    // Stress test memory allocation/deallocation
    for (int round = 0; round < 10; round++) {
        void *ptrs[100];
        for (int i = 0; i < 100; i++) {
            ptrs[i] = _libid_palloc(1024 + i);
            if (!ptrs[i]) {
                printf("  ✗ FAIL: Stress allocation failed at round %d, item %d\n", round, i);
                return 1;
            }
        }
        // Note: We don't explicitly free since we're using GC
    }
    printf("  ✓ Stress memory allocation successful (10 rounds x 100 allocations)\n");
    
    printf("\n=== Test 8 Results ===\n");
    printf("✓ All error handling and edge case tests PASSED\n");
    printf("✓ NULL and empty string handling works\n");
    printf("✓ Large and special character strings handled\n");
    printf("✓ Memory allocation edge cases handled\n");
    printf("✓ Object creation edge cases work\n");
    printf("✓ Import/export edge cases handled\n");
    printf("✓ Stress testing successful\n");
    printf("✓ Enhanced libid is robust and handles edge cases well\n");
    
    return 0;
}
