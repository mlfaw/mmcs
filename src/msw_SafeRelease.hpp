/*{REPLACEMEWITHLICENSE}*/
#pragma once

namespace msw {

template <class T>
void SafeRelease(T ** x) {
    if (*x) {
        (*x)->Release();
        *x = nullptr;
    }
}

}
