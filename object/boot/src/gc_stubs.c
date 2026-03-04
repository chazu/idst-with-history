/* GC stub functions for NO_GC build under Rosetta 2.
 * Boehm GC 7.0 crashes under Rosetta 2 due to memory layout
 * differences, so we stub out all GC functions and use plain
 * malloc/calloc instead (via libid.c's NO_GC path).
 */

#include <stdlib.h>

void GC_init(void) {}
void GC_gcollect(void) {}
void GC_add_roots(void *low, void *high) {}
void GC_unregister_disappearing_link(void **link) {}
int  GC_general_register_disappearing_link(void **link, void *obj) { return 0; }
long GC_get_free_bytes(void) { return 0; }

void *GC_malloc(size_t size) {
    return calloc(1, size);
}

void *GC_malloc_atomic(size_t size) {
    return calloc(1, size);
}

void *GC_realloc(void *ptr, size_t size) {
    return realloc(ptr, size);
}

void GC_free(void *ptr) {
    free(ptr);
}
