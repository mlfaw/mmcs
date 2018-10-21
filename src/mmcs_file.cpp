/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_file.hpp"

#ifdef _WIN32
//#include <shlwapi.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace mmcs {

File::File()
	: handle_((osfile)-1)
{
}

File::File(const oschar * fileName, const char * flags) // default flags of "r"
{
#ifdef _WIN32
	DWORD access = 0;
	//DWORD creationDisposition = 0;
	if (strchr(flags, 'r'))
		access |= GENERIC_READ;
	if (strchr(flags, 'w'))
		access |= GENERIC_WRITE;
	HANDLE hFile = CreateFileW(
		fileName,
		access,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE) {
		handle_ = (osfile)-1;
		return;
	}
	handle_ = hFile;
#else
	int flags = 0;
	if (strchr(flags, 'r') && strchr(flags, 'w'))
		flags = O_RDWR;
	else if (strchr(flags, 'r'))
		flags = O_RDONLY;
	else if (strchr(flags, 'w'))
		flags = O_WRONLY;
	handle_ = open(fileName, flags);
#endif
}

File::~File()
{
	// Close handle?
}

bool File::isValid()
{
	return handle_ != (osfile)-1;
}

File File::Invalid()
{
	File file;
	return file;
}

bool File::isDirectory()
{
	if (!isValid())
		return false;
#ifdef _WIN32
	BY_HANDLE_FILE_INFORMATION info;
	BOOL bRes = GetFileInformationByHandle(handle_, &info);
	if (!bRes) return false; // throw exception?
	return !!(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
#else
	struct stat info;
	if (fstat(handle_, &info) != 0) return false; // throw exception?
	return S_ISDIR(info.st_mode);
#endif
}

File File::openContainingDirectory(const oschar * fileName, const char * flags)
{
	size_t len = osstrlen(fileName);
	// Minimum of 3 for "a/b"
	if (!len || len < 3) return File::Invalid(); // TODO: hmm...

	const oschar * lastSeparator = osstrrchr(fileName, _OS('/'));
#ifdef _WIN32
	{
		const oschar * lastSeparator2 = osstrrchr(fileName, L'\\');
		if (lastSeparator < lastSeparator2)
			lastSeparator = lastSeparator2;
	}
#endif

	if (!lastSeparator || lastSeparator == fileName)
		return File::Invalid(); // TODO: log error???

	size_t charCount = lastSeparator - fileName;
	oschar * dup = (oschar *)malloc((charCount + 1) * sizeof(oschar));
	if (!dup) return File::Invalid(); // TODO: log error??

	(void)memcpy(dup, fileName, sizeof(oschar) * charCount);
	dup[charCount] = 0;

	auto ret = File(dup, flags);

	free(dup);
	return ret;
}

bool File::getSize(uint64_t * outsize)
{
	if (!isValid())
		return false;
#ifdef _WIN32
	LARGE_INTEGER li;
	if (!GetFileSizeEx(handle_, &li)) return false;
	*outsize = (uint64_t)li.QuadPart;
#else
	struct stat info;
	if (fstat(handle_, &info) != 0) return false; // throw exception?
	*outsize = (uint64_t)info.st_size;
#endif
	return true;
}

bool File::myRead(void * buf, uint32_t size)
{
#ifdef _WIN32
	DWORD bytes_read;
	if (!ReadFile(handle_, buf, size, &bytes_read, NULL))
		return false;
	return size == bytes_read;
#else
	ssize_t x;
	while ((x = read(handle_, buf, size)) == -1 && errno == EINTR)
		;
	return x == size;
#endif
}

// Slurp and also add a zero byte at the end.
// Makes strings easier...
bool File::slurp(void ** outbuf, uint32_t * outsize)
{
	uint64_t size;
	if (!getSize(&size)) return false;
	if (size > 0xFFFFFFFE) return false;
	uint32_t newSize = (uint32_t)size;
	char * buf = (char *)malloc(newSize + 1);
	if (!buf) return false;
	if (!myRead(buf, newSize)) {
		free(buf);
		return false;
	}
	buf[newSize] = 0;
	*outsize = newSize;
	*outbuf = (void *)buf;
	return true;
}

} // namespace mmcs
