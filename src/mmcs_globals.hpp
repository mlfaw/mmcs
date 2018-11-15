/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include "mmcs_os.hpp"

namespace mmcs {

extern int argc;
extern oschar ** argv;
extern bool isPortable;

extern oschar * ExePath;
extern oschar * ExeDir;

#ifdef _WIN32
extern wchar_t OriginalWorkingDirectory[];
#endif

}
