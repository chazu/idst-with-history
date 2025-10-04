#include <stdio.h>
#include <stdlib.h>

// Forward declarations from libid
struct __libid;
struct __libid *_libid_init(int *argcp, char ***argvp, char ***envp);
void *_libid_intern(const char *string);
void *_libid_proto(void *base);

int main(int argc, char **argv, char **envp)
{
    printf("Testing enhanced libid on Apple Silicon ARM64...\n");
    
    // Initialize the libid system
    struct __libid *libid = _libid_init(&argc, &argv, &envp);
    if (!libid) {
        printf("ERROR: Failed to initialize libid\n");
        return 1;
    }
    
    printf("SUCCESS: libid initialized successfully!\n");
    
    // Test basic selector interning
    void *selector = _libid_intern("test");
    if (!selector) {
        printf("ERROR: Failed to intern selector\n");
        return 1;
    }
    
    printf("SUCCESS: Selector interning works!\n");
    
    // Test prototype creation
    void *proto = _libid_proto(0);
    if (!proto) {
        printf("ERROR: Failed to create prototype\n");
        return 1;
    }
    
    printf("SUCCESS: Prototype creation works!\n");
    
    printf("All tests passed! Enhanced libid is working on Apple Silicon.\n");
    return 0;
}
