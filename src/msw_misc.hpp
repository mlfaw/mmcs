/*{REPLACEMEWITHLICENSE}*/
#pragma once

// UWP can be checked with #if !WINAPI_PARTITION_DESKTOP

// Excluding NULL-terminator.
#define NT_MAX_PATH (32767)

namespace msw {

template <class T>
void SafeRelease(T ** x) {
    if (*x) {
        (*x)->Release();
        *x = nullptr;
    }
}

}
