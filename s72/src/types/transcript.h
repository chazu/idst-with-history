#ifndef S72_TRANSCRIPT_H
#define S72_TRANSCRIPT_H

#include "../s72.h"

// Transcript singleton structure (no instance data needed)
typedef struct {
    // No instance data - Transcript is a singleton
} S72Transcript;

// Transcript type functions
void s72_transcript_init(void);
S72Value s72_transcript_singleton(void);

// Transcript methods (native implementations)
oop s72_transcript_print(oop closure, oop state, oop receiver, ...);
oop s72_transcript_cr(oop closure, oop state, oop receiver);
oop s72_transcript_show(oop closure, oop state, oop receiver, ...);

#endif // S72_TRANSCRIPT_H
