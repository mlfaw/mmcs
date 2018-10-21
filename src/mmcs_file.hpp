/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include "mmcs_os.hpp"
#include <string>
#include <vector>
#include <stdint.h>
#ifdef MMCS_MSW
#include <Windows.h>
#endif

namespace mmcs {

class File {
public:
	File();
	// Opens in read-mode.
	File(const oschar * fileName, const char * flags = "r");
	~File();

	bool isValid();

	bool isDirectory();

	static File Invalid();
	static File openContainingDirectory(const oschar * fileName, const char * flags = "r");

	bool getSize(uint64_t * outsize);

	bool myRead(void * buf, uint32_t size);

	bool slurp(void ** outbuf, uint32_t * outsize);

public:
	osfile handle_;
};

} // namespace mmcs
