#include "utils.h"

u32 skip_whitespace(cstr *input) {
    u32 count = 0;
    char *iter = cstr_begin(input);
    while(iter != cstr_end(input) && (*iter == ' ' || *iter == '\t')) {
        ++iter;
        ++count;
    }
    return count;
}