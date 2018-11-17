/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include "mmcs_os.hpp"
#include <string>
#include <vector>
#include <stdint.h>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace mmcs {
namespace file {

void closeFile(osfile f);

/* options for `flags`:
generic:
'r' - read
'w' - write
'd' - file to be a directory
'o' - open the file (default)
'c' - create the file
'y' - always-open / always-create

'M' - minimum opening... set O_PATH for linux and dwDesiredAccess=0 for Windows
      useful for seeing if a file exists if there's no intention of further IO on it

posix:
'E' - DO NOT add the O_CLOEXEC flag on Linux (BSDs too?)
'P' - add the O_PATH flag... fchdir(), fstat(), openat() / *at()
TODO: Mode presets...

windows:
'V' - overlapped (FILE_FLAG_OVERLAPPED) IO for Windows
'R' - FILE_SHARE_READ  (default)
'W' - FILE_SHARE_WRITE (default)
'D' - FILE_SHARE_DELETE
'S' - FILE_FLAG_SEQUENTIAL_SCAN
*/
osfile simpleOpen(const oschar * fileName, const char * flags);
osfile simpleRelativeOpen(osfile dir, const oschar * fileName, const char * flags);

bool isDirectory(osfile f);

bool getSize(osfile f, uint64_t * outsize);

bool simpleRead(osfile f, void * buf, uint32_t size);

// free() the result...
char * slurp(osfile f, uint32_t * outsize);

// free() the result...
oschar * getDir(const oschar * path);

}
}
