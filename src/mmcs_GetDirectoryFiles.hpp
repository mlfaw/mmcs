/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include "mmcs_os.hpp"
#include <vector>
#include <string>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace mmcs {

bool GetDirectoryFilesFromHandle(std::vector<osstring> ** results, osfile in_dir);

// Returns relative file-names from dirName
bool GetDirectoryFiles(std::vector<osstring> ** results, const oschar * dirName);

}
