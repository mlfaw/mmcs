/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include "mmcs_os.hpp" // oschar

namespace mmcs {

void OpenUrlSync(const char * url);
bool OpenUrlAsync(const char * url, bool FreeAfter);

void OpenFileSync(const oschar * url);
bool OpenFileAsync(const oschar * url, bool FreeAfter);

}
