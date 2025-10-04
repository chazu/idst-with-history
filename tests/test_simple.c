/*
 * Simple test that doesn't call _libid_init to isolate the issue
 */

#include <stdio.h>
#include <stdlib.h>

// Test that we can link with the enhanced libid without initializing
extern void *_libid_palloc(size_t size);
extern void *_libid_balloc(size_t size);

int main()
{
    printf("=== Simple Link Test ===\n");
    printf("Testing that enhanced libid links correctly...\n");
    
    // Test that the functions exist (don't call them, just reference them)
    printf("_libid_palloc function address: %p\n", (void*)_libid_palloc);
    printf("_libid_balloc function address: %p\n", (void*)_libid_balloc);
    
    printf("SUCCESS: Enhanced libid links properly!\n");
    printf("The issue is likely in _libid_init, not in the basic linking.\n");
    
    return 0;
}
