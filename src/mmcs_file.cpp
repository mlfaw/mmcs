/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_file.hpp"

#if MMCS_WIN32
//#include <shlwapi.h>
#include <winternl.h> // NtCreateFile(), RtlInitUnicodeString(), InitializeObjectAttributes()
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace mmcs {
namespace file {

void close(osfile f)
{
#if MMCS_WIN32
	(void)CloseHandle(f);
#else
	close(f); // TODO: handle -EINTR....
#endif
}

#if MMCS_WIN32
static bool parse_flags_win32(
	const char * in_flags,
	DWORD * out_access,
	DWORD * out_shareMode,
	DWORD * out_creationDisposition,
	DWORD * out_flagsAndAttr
)
{
	DWORD access = 0;
	DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
	DWORD creationDisposition = OPEN_EXISTING;
	DWORD flagsAndAttr = FILE_ATTRIBUTE_NORMAL;
	if (strchr(in_flags, 'r'))
		access |= GENERIC_READ;
	if (strchr(in_flags, 'w'))
		access |= GENERIC_WRITE;
	if (strchr(in_flags, 'd'))
		flagsAndAttr |= FILE_FLAG_BACKUP_SEMANTICS;

	// Directories can only be created with CreateDirectory.... or with NtCreateFile(FILE_DIRECTORY_FILE)...
	// TODO: deal with this I guess....
	if (creationDisposition != OPEN_EXISTING && (flagsAndAttr & FILE_FLAG_BACKUP_SEMANTICS))
		return false;

	*out_access = access;
	*out_shareMode = shareMode;
	*out_creationDisposition = creationDisposition;
	*out_flagsAndAttr = flagsAndAttr;
	return true;
}
#else
static bool parse_flags_posix(
	const char * in_flags,
	int * out_flags,
	int * out_mode
)
{
	int flags = 0;
	int mode = 0;
	if (strchr(in_flags, 'r') && strchr(in_flags, 'w'))
		flags = O_RDWR;
	else if (strchr(in_flags, 'r'))
		flags = O_RDONLY;
	else if (strchr(in_flags, 'w'))
		flags = O_WRONLY;
	*out_flags = flags;
	*out_mode = mode;
	return true;
}
#endif

osfile simpleOpen(const oschar * fileName, const char * flags) // TODO: default flags of "r"
{
#if MMCS_WIN32
	DWORD access, shareMode, creationDisposition, flagsAndAttr;
	if (!parse_flags_win32(flags, &access, &shareMode, &creationDisposition, &flagsAndAttr))
		return INVALID_HANDLE_VALUE;
	return CreateFileW(
		fileName,
		access,
		shareMode,
		NULL,
		creationDisposition,
		flagsAndAttr,
		NULL
	);
#else
	int oflags, mode;
	if (!parse_flags_posix(&oflags, mode))
		return -1;
	return open(fileName, oflags, mode);
#endif
}

osfile simpleRelativeOpen(osfile dir, const oschar * fileName, const char * flags)
{
#if MMCS_WIN32
	DWORD access, shareMode, creationDisposition, flagsAndAttr;
	if (!parse_flags_win32(flags, &access, &shareMode, &creationDisposition, &flagsAndAttr))
		return INVALID_HANDLE_VALUE;

	// Why did Microsoft use different Nt & Win32 values for these options...
	if (creationDisposition == CREATE_ALWAYS)
		creationDisposition = FILE_OVERWRITE_IF;
	else if (creationDisposition == CREATE_NEW)
		creationDisposition = FILE_CREATE;
	else if (creationDisposition == OPEN_ALWAYS)
		creationDisposition = FILE_OPEN_IF;
	else if (creationDisposition == OPEN_EXISTING)
		creationDisposition = FILE_OPEN;
	else if (creationDisposition == TRUNCATE_EXISTING)
		creationDisposition = FILE_OVERWRITE;
	else
		return INVALID_HANDLE_VALUE;

	DWORD attr = 0;
	// TODO: handle attr

	DWORD createOptions = 0;
	if (flagsAndAttr & FILE_FLAG_BACKUP_SEMANTICS)
		createOptions |= FILE_DIRECTORY_FILE;
	else
		createOptions |= FILE_NON_DIRECTORY_FILE;
	
	UNICODE_STRING us;
	RtlInitUnicodeString(&us, fileName);

	OBJECT_ATTRIBUTES objattr;
	InitializeObjectAttributes(
		&objattr,
		&us,
		OBJ_CASE_INSENSITIVE,
		dir,
		NULL // security descriptor... use default
	);

	HANDLE hFile;
	IO_STATUS_BLOCK statusblock;
	NTSTATUS ntstatus = NtCreateFile(
		&hFile,
		access,
		&objattr,
		&statusblock,
		0, // AllocationSize
		attr,
		shareMode,
		creationDisposition,
		createOptions,
		NULL, // EaBuffer
		0	  // EaLength
	);

	if (!NT_SUCCESS(ntstatus)) {
		//if (statusblock.Information != FILE_DOES_NOT_EXIST)
			// TODO: Log error?
		return INVALID_HANDLE_VALUE; 
	}

	return hFile;
#else
	int oflags, mode;
	if (!parse_flags_posix(&oflags, mode))
		return -1;
	return openat(dir, fileName, oflags, mode);
#endif
}

bool isDirectory(osfile f)
{
#ifdef MMCS_WIN32
	BY_HANDLE_FILE_INFORMATION info;
	BOOL bRes = GetFileInformationByHandle(f, &info);
	if (!bRes) return false;
	return !!(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
#else
	struct stat info;
	if (fstat(f, &info) != 0) return false;
	return S_ISDIR(info.st_mode);
#endif
}

#if 0
File File::openContainingDirectory(const oschar * fileName, const char * flags)
{
	size_t len = osstrlen(fileName);
	// Minimum of 3 for "a/b"
	if (!len || len < 3) return File::Invalid(); // TODO: hmm...

	const oschar * lastSeparator = osstrrchr(fileName, _OS('/'));
#ifdef MMCS_WIN32
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
#endif

bool getSize(osfile f, uint64_t * outsize)
{
#ifdef MMCS_WIN32
	LARGE_INTEGER li;
	if (!GetFileSizeEx(f, &li)) return false;
	*outsize = (uint64_t)li.QuadPart;
#else
	struct stat info;
	if (fstat(f, &info) != 0) return false;
	*outsize = (uint64_t)info.st_size;
#endif
	return true;
}

// TODO: Support 64-bit sizes? Who the fuck would be reading 4 GiB files though...
bool simpleRead(osfile f, void * buf, uint32_t size)
{
#ifdef MMCS_WIN32
	DWORD bytes_read;
	if (!ReadFile(f, buf, size, &bytes_read, NULL))
		return false;
	return size == bytes_read;
#else
	ssize_t x;
	while ((x = read(f, buf, size)) == -1 && errno == EINTR)
		;
	return x == size;
#endif
}

// Slurp and also add a zero byte at the end.
// Makes strings easier...
bool slurp(osfile f, void ** outbuf, uint32_t * outsize)
{
	uint64_t size;
	if (!getSize(f, &size)) return false;
	if (size > 0xFFFFFFFE) return false;
	uint32_t newSize = (uint32_t)size;
	char * buf = (char *)malloc(newSize + 1);
	if (!buf) return false;
	if (!simpleRead(f, buf, newSize)) {
		free(buf);
		return false;
	}
	buf[newSize] = 0;
	*outsize = newSize;
	*outbuf = (void *)buf;
	return true;
}

} // namespace file
} // namespace mmcs
