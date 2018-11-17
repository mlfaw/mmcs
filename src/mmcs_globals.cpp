/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_globals.hpp"

namespace mmcs {

bool isPortable = false;

oschar * ExeDir = nullptr;

#ifdef _WIN32
wchar_t * OriginalWorkingDirectory = nullptr;
#endif

}
