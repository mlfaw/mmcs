/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include "mmcs_os.hpp"

namespace mmcs {

// The names for these functions are ugly....
bool osStringInArray(const oschar * in, const oschar ** array, int count);
bool osStringEndsWithExtension(const oschar * in, const oschar * ext);

}
