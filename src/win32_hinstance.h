// https://blogs.msdn.microsoft.com/oldnewthing/20041025-00/?p=37483
// "Accessing the current module's HINSTANCE from a static library"
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <Windows.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

#ifdef __cplusplus
}
#endif
