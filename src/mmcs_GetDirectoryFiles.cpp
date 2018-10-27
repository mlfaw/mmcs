/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_GetDirectoryFiles.hpp"

#if MMCS_WIN32
#include <stdlib.h> // malloc(), free()
#include <string.h> // memcpy()
#include <Windows.h>

#include <winternl.h>
typedef struct _FILE_NAMES_INFORMATION {
	ULONG NextEntryOffset;
	ULONG FileIndex;
	ULONG FileNameLength;
	WCHAR FileName[1];
} FILE_NAMES_INFORMATION, *PFILE_NAMES_INFORMATION;

 extern "C"
 NTSYSCALLAPI
 NTSTATUS
 NTAPI
 NtQueryDirectoryFile(
 	_In_ HANDLE FileHandle,
 	_In_opt_ HANDLE Event,
 	_In_opt_ PIO_APC_ROUTINE ApcRoutine,
 	_In_opt_ PVOID ApcContext,
 	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
 	_Out_writes_bytes_(Length) PVOID FileInformation,
 	_In_ ULONG Length,
 	_In_ FILE_INFORMATION_CLASS FileInformationClass,
 	_In_ BOOLEAN ReturnSingleEntry,
 	_In_opt_ PUNICODE_STRING FileName,
 	_In_ BOOLEAN RestartScan
 );
#else
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#endif

namespace mmcs {

bool GetDirectoryFilesFromHandle(std::vector<osstring> ** results, osfile in_hDir)
{
	bool success = false;
	std::vector<osstring> * files = NULL;
	HANDLE hDir;

	// Duplicate in_hDir as NtQueryDirectoryFile() might(?) clobber a scan?
	{
		HANDLE proc = GetCurrentProcess();
		if (!DuplicateHandle(proc, in_hDir, proc, &hDir, 0, FALSE, DUPLICATE_SAME_ACCESS))
			return false;
	}

	char unaligned_buf[1024 * 16];
	// LONG alignment is a requirement for FileNamesInformation
	char * long_aligned_buf = (char *)((UINT_PTR)unaligned_buf & ~(UINT_PTR)(sizeof(LONG)));
	ULONG length = (1024 * 16) - (long_aligned_buf - unaligned_buf);

	IO_STATUS_BLOCK isb;
	NTSTATUS status = NtQueryDirectoryFile(
		hDir,
		NULL, // Event
		NULL, // ApcRoutine
		NULL, // ApcContext
		&isb,
		long_aligned_buf,
		length,
		(FILE_INFORMATION_CLASS)12, // FileNamesInformation
		FALSE, // ReturnSingleEntry
		NULL, // FileName
		TRUE // RestartScan
	);

	if (status != 0) // STATUS_SUCCESS is 0
		goto out;

	try {
		files = new std::vector<osstring>();
	} catch (...) {
		goto out;
	}

	do {
		PFILE_NAMES_INFORMATION names = (PFILE_NAMES_INFORMATION)long_aligned_buf;
		ULONG nextEntryOffset = 0;

		do {
			auto nchars = names->FileNameLength / sizeof(wchar_t);
			auto filename = names->FileName;
			nextEntryOffset = names->NextEntryOffset;

			// skip . & ..
			if (filename[0] == L'.')
			{
				if (nchars == 1)
					continue;
				if (nchars == 2 && filename[1] == L'.')
					continue;
			}

			try {
				files->push_back(osstring(filename, nchars));
			} catch (...) {
				goto out;
			}
		} while (nextEntryOffset && (names = (PFILE_NAMES_INFORMATION)((char *)names + nextEntryOffset)));

		status = NtQueryDirectoryFile(
			hDir,
			NULL, // Event
			NULL, // ApcRoutine
			NULL, // ApcContext
			&isb,
			long_aligned_buf,
			length,
			(FILE_INFORMATION_CLASS)12, // FileNamesInformation
			FALSE, // ReturnSingleEntry
			NULL, // FileName
			FALSE // RestartScan
		);
	} while (status == 0 && isb.Information);

	// STATUS_SUCCESS or STATUS_NO_MORE_FILES
	if (status == 0 || status == 0x80000006L)
		success = true;

out:
	CloseHandle(hDir);
	if (success)
		*results = files;
	else if (files)
		delete files;
	return success;
}

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
