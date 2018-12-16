/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include "mmcs_os.h"

namespace mmcs {

extern bool isPortable;

extern oschar * ExeDir;

#ifdef _WIN32
extern wchar_t * OriginalWorkingDirectory;
#endif

}
