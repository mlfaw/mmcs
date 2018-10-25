/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_GetDirectoryFiles.hpp"

#if MMCS_WIN32
#include <stdlib.h> // malloc(), free()
#include <string.h> // memcpy()
#include <Windows.h>
// #include <winternl.h>

// extern "C"
// NTSYSCALLAPI
// NTSTATUS
// NTAPI
// NtQueryDirectoryFile(
// 	_In_ HANDLE FileHandle,
// 	_In_opt_ HANDLE Event,
// 	_In_opt_ PIO_APC_ROUTINE ApcRoutine,
// 	_In_opt_ PVOID ApcContext,
// 	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
// 	_Out_writes_bytes_(Length) PVOID FileInformation,
// 	_In_ ULONG Length,
// 	_In_ FILE_INFORMATION_CLASS FileInformationClass,
// 	_In_ BOOLEAN ReturnSingleEntry,
// 	_In_opt_ PUNICODE_STRING FileName,
// 	_In_ BOOLEAN RestartScan
// );
#else
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#endif

namespace mmcs {

// TODO: Use NtQueryDirectoryFile() on Windows and fdopendir() on Linux as they use a directory handle/fd.
//       On Windows that means we can remove the `dirName + "\*"` string allocation.
//       The caller can either use a pre-existing directory handle/fd or a wrapper to handle opening the directory.
bool GetDirectoryFiles(std::vector<osstring> ** results, const oschar * dirName)
{
	std::vector<osstring> * files = NULL;
#if MMCS_WIN32
	// (void)NtQueryDirectoryFile(0, 0, 0, 0, 0, 0, 0, FileDirectoryInformation, 0, 0, 0);
	auto x = wcslen(dirName);
	if (!x) return false;

	wchar_t * filter = (wchar_t *)malloc((x + 3) * sizeof(wchar_t));
	if (!filter) return false;

	memcpy(filter, dirName, x * sizeof(wchar_t));
	filter[x + 0] = L'\\';
	filter[x + 1] = L'*';
	filter[x + 2] = 0;
	//(void)PathAppendW(thing, L"\\*");

	WIN32_FIND_DATAW findData;
	HANDLE hFindFile = FindFirstFileExW(
		filter,
		FindExInfoBasic,
		&findData,
		FindExSearchNameMatch,
		NULL,
		FIND_FIRST_EX_LARGE_FETCH
	);

	free(filter);

	if (hFindFile == INVALID_HANDLE_VALUE)
		return false;

	try {
		files = new std::vector<osstring>();
	} catch (...) {
		(void)FindClose(hFindFile);
		return false;
	}

	do {
		auto filename = findData.cFileName;
		// skip files "." and ".."
		if (filename[0] == L'.')
		{
			int nulidx = 1;
			if (filename[1] == L'.')
				nulidx = 2;
			if (filename[nulidx] == 0)
				continue;
		}

		try {
			files->push_back(osstring(filename));
		} catch (...) {
			delete files;
			(void)FindClose(hFindFile);
			return false;
		}
	} while (FindNextFileW(hFindFile, &findData));

	(void)FindClose(hFindFile);
	*results = files;
	return true;
#else
	DIR * dir = opendir(dirName);
	if (!dir) return false;
	struct dirent * data;
	while ((data = readdir(dir)) != NULL) {
		try {
			files->push_back(osstring(data->d_name));
		} catch (...) {
			delete files;
			(void)closedir(dir);
			return false;
		}
	}
	(void)closedir(dir);

	*results = files;
	return true;
#endif
}

} // namespace mmcs
