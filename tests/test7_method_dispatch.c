/*
 * Test 7: Method Dispatch and Calling
 * Tests that numbered macros actually work for method calls (not just compilation)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// Include the full libid system
#include "../object/stage1/include/id/id.h"

// Static libid instance (like generated code)
static struct __libid *_libid= 0;

// Include the enhanced macros
#include "../object/id/libid_enhanced.h"

// Function declarations
struct __libid *_libid_init(int *argcp, char ***argvp, char ***envp);

// Test methods that return distinctive values (using correct signature for _send macros)
oop test_method_0(oop closure, oop self, oop receiver)
{
    printf("    → test_method_0 called on %p (closure=%p, self=%p)\n", receiver, closure, self);
    return (oop)0x1000;  // Distinctive return value
}

oop test_method_1(oop closure, oop self, oop receiver, oop arg1)
{
    printf("    → test_method_1 called on %p with arg %p (closure=%p, self=%p)\n", receiver, arg1, closure, self);
    return (oop)0x1001;  // Distinctive return value
}

oop test_method_2(oop closure, oop self, oop receiver, oop arg1, oop arg2)
{
    printf("    → test_method_2 called on %p with args %p, %p (closure=%p, self=%p)\n", receiver, arg1, arg2, closure, self);
    return (oop)0x1002;  // Distinctive return value
}

oop test_method_3(oop closure, oop self, oop receiver, oop arg1, oop arg2, oop arg3)
{
    printf("    → test_method_3 called on %p with args %p, %p, %p (closure=%p, self=%p)\n", receiver, arg1, arg2, arg3, closure, self);
    return (oop)0x1003;  // Distinctive return value
}

int main(int argc, char **argv, char **envp)
{
    printf("=== Test 7: Method Dispatch and Calling ===\n");

    // Initialize libid (like generated code does)
    struct __libid *libid = _libid_init(&argc, &argv, &envp);
    if (!libid) {
        printf("FAIL: Could not initialize libid\n");
        return 1;
    }

    // Initialize our static _libid variable (like generated code does)
    if (!(_libid= libid)) {
        printf("FAIL: Could not set static _libid\n");
        return 1;
    }
    printf("✓ libid initialized successfully\n");
    
    // Create test object
    oop obj = _libid->proto(0);
    if (!obj) {
        printf("FAIL: Could not create test object\n");
        return 1;
    }
    printf("✓ Test object created: %p\n", obj);

    // Create selectors
    oop sel0 = _libid->intern("test0");
    oop sel1 = _libid->intern("test1:");
    oop sel2 = _libid->intern("test2:with:");
    oop sel3 = _libid->intern("test3:with:and:");

    if (!sel0 || !sel1 || !sel2 || !sel3) {
        printf("FAIL: Could not create selectors\n");
        return 1;
    }
    printf("✓ Test selectors created\n");

    // Install test methods
    printf("✓ Installing test methods...\n");
    _libid->method(obj, sel0, (_imp_t)test_method_0);
    _libid->method(obj, sel1, (_imp_t)test_method_1);
    _libid->method(obj, sel2, (_imp_t)test_method_2);
    _libid->method(obj, sel3, (_imp_t)test_method_3);
    printf("  - All test methods installed\n");
    
    // Test actual method dispatch using _send macros
    printf("✓ Testing _send macro dispatch...\n");

    // Test 0-argument method dispatch
    printf("  Testing _send0 macro:\n");
    oop result0 = _send0(sel0, obj);
    if (result0 == (oop)0x1000) {
        printf("  ✓ PASS: _send0 macro works correctly\n");
    } else {
        printf("  ✗ FAIL: _send0 macro failed, got %p expected %p\n", result0, (oop)0x1000);
        return 1;
    }

    // Test 1-argument method dispatch
    printf("  Testing _send1 macro:\n");
    oop result1 = _send1(sel1, obj, (oop)0xAAA);
    if (result1 == (oop)0x1001) {
        printf("  ✓ PASS: _send1 macro works correctly\n");
    } else {
        printf("  ✗ FAIL: _send1 macro failed, got %p expected %p\n", result1, (oop)0x1001);
        return 1;
    }

    // Test 2-argument method dispatch
    printf("  Testing _send2 macro:\n");
    oop result2 = _send2(sel2, obj, (oop)0xBBB, (oop)0xCCC);
    if (result2 == (oop)0x1002) {
        printf("  ✓ PASS: _send2 macro works correctly\n");
    } else {
        printf("  ✗ FAIL: _send2 macro failed, got %p expected %p\n", result2, (oop)0x1002);
        return 1;
    }

    // Test 3-argument method dispatch
    printf("  Testing _send3 macro:\n");
    oop result3 = _send3(sel3, obj, (oop)0xDDD, (oop)0xEEE, (oop)0xFFF);
    if (result3 == (oop)0x1003) {
        printf("  ✓ PASS: _send3 macro works correctly\n");
    } else {
        printf("  ✗ FAIL: _send3 macro failed, got %p expected %p\n", result3, (oop)0x1003);
        return 1;
    }

    // Test _sendv macros as well
    printf("✓ Testing _sendv macro dispatch...\n");

    // Test 0-argument _sendv dispatch
    printf("  Testing _sendv0 macro:\n");
    oop sendv_result0 = _sendv0(sel0, obj);
    if (sendv_result0 == (oop)0x1000) {
        printf("  ✓ PASS: _sendv0 macro works correctly\n");
    } else {
        printf("  ✗ FAIL: _sendv0 macro failed, got %p expected %p\n", sendv_result0, (oop)0x1000);
        return 1;
    }

    // Test 1-argument _sendv dispatch
    printf("  Testing _sendv1 macro:\n");
    oop sendv_result1 = _sendv1(sel1, obj, (oop)0xAAA);
    if (sendv_result1 == (oop)0x1001) {
        printf("  ✓ PASS: _sendv1 macro works correctly\n");
    } else {
        printf("  ✗ FAIL: _sendv1 macro failed, got %p expected %p\n", sendv_result1, (oop)0x1001);
        return 1;
    }

    // Test that we can also call the methods directly (for comparison)
    printf("✓ Testing direct method calls (for comparison):\n");
    oop direct_result0 = test_method_0(0, obj, obj);
    if (direct_result0 == (oop)0x1000) {
        printf("  ✓ PASS: Direct 0-arg method call works\n");
    } else {
        printf("  ✗ FAIL: Direct 0-arg method call failed\n");
        return 1;
    }
    
    // Test method installation on multiple objects
    printf("✓ Testing method installation on multiple objects...\n");
    for (int i = 0; i < 5; i++) {
        oop test_obj = _libid->proto(0);
        _libid->method(test_obj, sel0, (_imp_t)test_method_0);

        // Test that method was installed by using _send0 macro
        oop result = _send0(sel0, test_obj);
        if (result != (oop)0x1000) {
            printf("  ✗ FAIL: Method dispatch failed on object %d\n", i);
            return 1;
        }
    }
    printf("  ✓ Method installation and dispatch works on multiple objects\n");

    // Test method inheritance/delegation
    printf("✓ Testing method inheritance structure...\n");
    oop child_obj = _libid->proto(obj);  // Create child that delegates to obj
    if (!child_obj) {
        printf("FAIL: Could not create child object\n");
        return 1;
    }

    // Test that child object was created properly
    if (child_obj != obj) {
        printf("  ✓ PASS: Child object is different from parent\n");
    } else {
        printf("  ✗ FAIL: Child object is same as parent\n");
        return 1;
    }

    // Test that child inherits parent's methods via delegation
    printf("  Testing method inheritance via _send0:\n");
    oop inherited_result = _send0(sel0, child_obj);
    if (inherited_result == (oop)0x1000) {
        printf("  ✓ PASS: Child object inherits parent methods via delegation\n");
    } else {
        printf("  ✗ FAIL: Child object does not inherit parent methods\n");
        return 1;
    }
    
    printf("\n=== Test 7 Results ===\n");
    printf("✓ All method dispatch tests PASSED\n");
    printf("✓ _send macros work correctly - core messaging system functional!\n");
    printf("✓ _sendv macros work correctly - alternative dispatch functional!\n");
    printf("✓ Method installation is functional\n");
    printf("✓ Object delegation/inheritance works\n");
    printf("✓ Multiple object method dispatch successful\n");
    printf("✓ ARM64 method dispatch system verified\n");
    printf("✓ Id language messaging system is now fully operational!\n");
    
    return 0;
}
