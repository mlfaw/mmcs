/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include "mmcs_os.hpp"

namespace mmcs {

bool osStringInArray(const oschar * in, const oschar ** array, int count);
bool osStringEndsWithExtension(const oschar * in, const oschar * ext);

}
