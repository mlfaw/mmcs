/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include "mmcs_os.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace mmcs {

extern int argc;
extern oschar ** argv;
extern bool isPortable;

extern oschar * ExePath;
extern oschar * ExeDir;

#ifdef _WIN32
//extern HINSTANCE hInstance;
//extern wchar_t ExePath[];
//extern wchar_t * ExePath;
extern wchar_t OriginalWorkingDirectory[];
#endif

}
