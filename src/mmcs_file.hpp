/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include "mmcs_os.hpp"
#include <string>
#include <vector>
#include <stdint.h>
#if MMCS_MSW
#include <Windows.h>
#endif

namespace mmcs {
namespace file {

void close(osfile f);

osfile simpleOpen(const oschar * fileName, const char * flags);

osfile simpleRelativeOpen(osfile dir, const oschar * fileName, const char * flags);

bool isValid(osfile f);

bool isDirectory(osfile f);

bool getSize(osfile f, uint64_t * outsize);

bool simpleRead(osfile f, void * buf, uint32_t size);

bool slurp(osfile f, void ** outbuf, uint32_t * outsize);

}
} // namespace mmcs
