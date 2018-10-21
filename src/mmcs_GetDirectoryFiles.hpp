/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include "mmcs_os.hpp"
#include <vector>
#include <string>

namespace mmcs {

// Returns relative file-names from dirName
bool GetDirectoryFiles(std::vector<osstring> ** results, const oschar * dirName);

} // namespace mmcs
