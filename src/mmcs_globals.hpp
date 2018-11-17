/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include "mmcs_os.hpp"

namespace mmcs {

extern bool isPortable;

extern oschar * ExeDir;

#ifdef _WIN32
extern wchar_t * OriginalWorkingDirectory;
#endif

}
