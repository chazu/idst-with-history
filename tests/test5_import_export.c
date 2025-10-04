/*
 * Test 5: Import/Export Test (Tests Module System)
 * Tests the global namespace functionality
 */

#include <stdio.h>
#include <stdlib.h>

// Forward declarations from libid
struct __libid;
typedef struct t__object *oop;

struct __libid *_libid_init(int *argcp, char ***argvp, char ***envp);
oop _libid_intern(const char *string);
oop _libid_proto(oop base);
void _libid_export(const char *key, oop value);
oop _libid_import(const char *key);

int main(int argc, char **argv, char **envp)
{
    printf("=== Test 5: Import/Export Test ===\n");
    
    // Initialize libid
    struct __libid *libid = _libid_init(&argc, &argv, &envp);
    if (!libid) {
        printf("FAIL: Could not initialize libid\n");
        return 1;
    }
    printf("✓ libid initialized successfully\n");
    
    // Create test objects to export
    oop obj1 = _libid_proto(0);
    oop obj2 = _libid_proto(0);
    oop obj3 = _libid_proto(obj1);  // obj3 delegates to obj1
    
    if (!obj1 || !obj2 || !obj3) {
        printf("FAIL: Could not create test objects\n");
        return 1;
    }
    printf("✓ Test objects created: obj1=%p, obj2=%p, obj3=%p\n", obj1, obj2, obj3);
    
    // Create some selectors to export
    oop sel1 = _libid_intern("testSelector1");
    oop sel2 = _libid_intern("anotherSelector");
    
    if (!sel1 || !sel2) {
        printf("FAIL: Could not create test selectors\n");
        return 1;
    }
    printf("✓ Test selectors created\n");
    
    // Test basic export/import
    printf("✓ Testing basic export/import...\n");
    
    _libid_export("testObject1", obj1);
    printf("  - Exported obj1 as 'testObject1'\n");
    
    _libid_export("testObject2", obj2);
    printf("  - Exported obj2 as 'testObject2'\n");
    
    _libid_export("testSelector", sel1);
    printf("  - Exported sel1 as 'testSelector'\n");
    
    // Import them back
    oop imported1 = _libid_import("testObject1");
    oop imported2 = _libid_import("testObject2");
    oop imported_sel = _libid_import("testSelector");
    
    if (!imported1 || !imported2 || !imported_sel) {
        printf("FAIL: Import failed\n");
        return 1;
    }
    
    // Verify they're the same objects
    if (obj1 == imported1) {
        printf("✓ PASS: Object 1 import/export identity preserved\n");
    } else {
        printf("✗ FAIL: Object 1 import/export identity lost\n");
        return 1;
    }
    
    if (obj2 == imported2) {
        printf("✓ PASS: Object 2 import/export identity preserved\n");
    } else {
        printf("✗ FAIL: Object 2 import/export identity lost\n");
        return 1;
    }
    
    if (sel1 == imported_sel) {
        printf("✓ PASS: Selector import/export identity preserved\n");
    } else {
        printf("✗ FAIL: Selector import/export identity lost\n");
        return 1;
    }
    
    // Test overwriting exports
    printf("✓ Testing export overwriting...\n");
    
    _libid_export("overwriteTest", obj1);
    oop first_import = _libid_import("overwriteTest");
    
    _libid_export("overwriteTest", obj2);  // Overwrite with obj2
    oop second_import = _libid_import("overwriteTest");
    
    if (first_import == obj1 && second_import == obj2) {
        printf("✓ PASS: Export overwriting works correctly\n");
    } else {
        printf("✗ FAIL: Export overwriting failed\n");
        return 1;
    }
    
    // Test multiple exports
    printf("✓ Testing multiple exports...\n");
    
    char key_buffer[32];
    oop objects[10];
    
    // Create and export 10 objects
    for (int i = 0; i < 10; i++) {
        objects[i] = _libid_proto(0);
        if (!objects[i]) {
            printf("FAIL: Could not create object %d\n", i);
            return 1;
        }
        
        snprintf(key_buffer, sizeof(key_buffer), "multiTest%d", i);
        _libid_export(key_buffer, objects[i]);
    }
    printf("  - Exported 10 objects with different keys\n");
    
    // Import them back and verify
    for (int i = 0; i < 10; i++) {
        snprintf(key_buffer, sizeof(key_buffer), "multiTest%d", i);
        oop imported = _libid_import(key_buffer);
        
        if (imported != objects[i]) {
            printf("✗ FAIL: Multi-export test failed at index %d\n", i);
            return 1;
        }
    }
    printf("  - All 10 objects imported correctly\n");
    
    // Test exporting complex names
    printf("✓ Testing complex export names...\n");
    
    _libid_export("complex.name.with.dots", obj3);
    _libid_export("name_with_underscores", obj1);
    _libid_export("name-with-dashes", obj2);
    _libid_export("CamelCaseName", sel1);
    
    oop complex1 = _libid_import("complex.name.with.dots");
    oop complex2 = _libid_import("name_with_underscores");
    oop complex3 = _libid_import("name-with-dashes");
    oop complex4 = _libid_import("CamelCaseName");
    
    if (complex1 == obj3 && complex2 == obj1 && complex3 == obj2 && complex4 == sel1) {
        printf("✓ PASS: Complex export names work correctly\n");
    } else {
        printf("✗ FAIL: Complex export names failed\n");
        return 1;
    }
    
    printf("\n=== Test 5 Results ===\n");
    printf("✓ All import/export tests PASSED\n");
    printf("✓ Global namespace functionality working\n");
    printf("✓ Object identity preserved across import/export\n");
    printf("✓ Export overwriting works correctly\n");
    printf("✓ Multiple exports and complex names supported\n");
    printf("✓ Module system foundation verified\n");
    
    return 0;
}
