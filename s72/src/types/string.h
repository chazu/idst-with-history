#ifndef STRING_H
#define STRING_H

#include "../s72.h"

// String object structure
typedef struct S72String {
    char *data;
    size_t length;
} S72String;

// String type initialization
void s72_string_init(void);

// String creation and testing
S72Value s72_string_new(const char *str);
S72Value s72_string_new_with_length(const char *str, size_t length);
bool s72_is_string(S72Value val);
const char *s72_string_data(S72Value val);
size_t s72_string_length(S72Value val);

// String methods (native implementations)
oop s72_string_print(struct __send *send, oop self, oop receiver);
oop s72_string_equals(struct __send *send, oop self, oop receiver, oop arg);
oop s72_string_length_method(struct __send *send, oop self, oop receiver);

#endif // STRING_H
