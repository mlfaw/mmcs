/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include "mmcs_os.hpp"

#if MMCS_WIN32
#include <Windows.h>
#endif

namespace mmcs {

extern int argc;
extern oschar ** argv;
extern bool isPortable;

extern oschar * ExePath;
extern oschar * ExeDir;

#if MMCS_WIN32
//extern HINSTANCE hInstance;
//extern wchar_t ExePath[];
//extern wchar_t * ExePath;
extern wchar_t OriginalWorkingDirectory[];
#endif

}
