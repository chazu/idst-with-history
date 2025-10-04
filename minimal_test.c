#include <stdio.h>

// Test that we can link with the enhanced libid
extern void *_libid_palloc(size_t size);
extern void *_libid_balloc(size_t size);

int main()
{
    printf("Testing enhanced libid compilation on Apple Silicon ARM64...\n");
    
    // Test that the functions exist (don't call them, just reference them)
    printf("_libid_palloc function address: %p\n", (void*)_libid_palloc);
    printf("_libid_balloc function address: %p\n", (void*)_libid_balloc);
    
    printf("SUCCESS: Enhanced libid links properly on Apple Silicon!\n");
    return 0;
}
