/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include "mmcs_os.hpp" // oschar

namespace mmcs {

//const unsigned char ASYNC_EXIT = 1; // maybe it'd be useful...
//const unsigned char ASYNC_URL = 2;
//const unsigned char ASYNC_FILE = 4;

void OpenUrlSync(const char * url);
bool OpenUrlAsync(const char * url, bool FreeAfter);

void OpenFileSync(const oschar * url);
bool OpenFileAsync(const oschar * url, bool FreeAfter);

}
