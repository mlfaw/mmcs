/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include <stdlib.h> // free()

namespace mmcs {

class ScopedFree {
public:
    void * value_;

    ScopedFree(void * x) : value_(x) {}
    ~ScopedFree() {
        free(value_);
    }
};

}