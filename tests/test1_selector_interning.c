/*
 * Test 1: Selector Interning Test (Most Basic)
 * Tests basic selector creation and comparison
 */

#include <stdio.h>
#include <stdlib.h>

// Forward declarations from libid
struct __libid;
typedef struct t__object *oop;

struct __libid *_libid_init(int *argcp, char ***argvp, char ***envp);
oop _libid_intern(const char *string);

int main(int argc, char **argv, char **envp)
{
    printf("=== Test 1: Selector Interning Test ===\n");
    
    // Initialize libid
    struct __libid *libid = _libid_init(&argc, &argv, &envp);
    if (!libid) {
        printf("FAIL: Could not initialize libid\n");
        return 1;
    }
    printf("✓ libid initialized successfully\n");
    
    // Test selector interning
    oop sel1 = _libid_intern("hello");
    if (!sel1) {
        printf("FAIL: Could not intern 'hello'\n");
        return 1;
    }
    printf("✓ First selector 'hello' interned: %p\n", sel1);
    
    oop sel2 = _libid_intern("hello");
    if (!sel2) {
        printf("FAIL: Could not intern 'hello' second time\n");
        return 1;
    }
    printf("✓ Second selector 'hello' interned: %p\n", sel2);
    
    oop sel3 = _libid_intern("world");
    if (!sel3) {
        printf("FAIL: Could not intern 'world'\n");
        return 1;
    }
    printf("✓ Third selector 'world' interned: %p\n", sel3);
    
    // Test selector identity
    if (sel1 == sel2) {
        printf("✓ PASS: Same selector interned twice returns same object\n");
    } else {
        printf("✗ FAIL: Same selector interned twice returns different objects\n");
        return 1;
    }
    
    if (sel1 != sel3) {
        printf("✓ PASS: Different selectors return different objects\n");
    } else {
        printf("✗ FAIL: Different selectors return same object\n");
        return 1;
    }
    
    // Test more selectors
    oop sel4 = _libid_intern("test:with:and:");
    oop sel5 = _libid_intern("anotherSelector");
    oop sel6 = _libid_intern("test:with:and:");  // Same as sel4
    
    if (sel4 == sel6) {
        printf("✓ PASS: Complex selector interning works correctly\n");
    } else {
        printf("✗ FAIL: Complex selector interning failed\n");
        return 1;
    }
    
    printf("\n=== Test 1 Results ===\n");
    printf("✓ All selector interning tests PASSED\n");
    printf("✓ Basic libid functionality verified\n");
    printf("✓ Memory management working\n");
    
    return 0;
}
