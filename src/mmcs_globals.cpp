/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_globals.hpp"
#include "msw_misc.hpp"

namespace mmcs {

int argc = 0;
oschar ** argv = NULL;
bool isPortable = false;

oschar * ExePath = NULL;
oschar * ExeDir = NULL;

#if MMCS_WIN32
//HINSTANCE hInstance = NULL;
//wchar_t ExePath[NT_MAX_PATH + 1];
//wchar_t * ExePath = NULL;
wchar_t OriginalWorkingDirectory[NT_MAX_PATH + 1];
#endif

}