#ifndef STRING_H
#define STRING_H

#include "../s72.h"

// Pure libid string object structure
typedef struct t_String {
    oop length;    // Number object for string length
    oop data;      // libid-allocated string data (char array)
} *String;

// String type initialization
void s72_string_init(void);

// String creation and testing
S72Value s72_string_new(const char *str);
S72Value s72_string_new_with_length(const char *str, size_t length);
bool s72_is_string(S72Value val);
const char *s72_string_data(S72Value val);
size_t s72_string_length(S72Value val);

// String methods (pure libid implementations with variadic signatures)
oop s72_string_print(oop closure, oop state, oop self);
oop s72_string_equals(oop closure, oop state, oop self, ...);
oop s72_string_length_method(oop closure, oop state, oop self);

// String object creation
oop s72_string_new_libid(const char *str, size_t length);

#endif // STRING_H
