/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_StringHelpers.hpp"
#include <string.h>

namespace mmcs {

bool osStringInArray(const oschar * in, const oschar ** array, int count)
{
	for (int i = 0; i < count; ++i) {
		if (osstrcmp(array[i], in) == 0)
			return true;
	}
	return false;
}

bool osStringEndsWithExtension(const oschar * in, const oschar * ext)
{
	in = osstrrchr(in, _OS('.'));
	return in && (osstrcmp(in, ext) == 0);
}

}
