/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_os.hpp"
#include "mmcs_globals.hpp"
#include "mmcs_NativeMessaging.hpp"
#include <string.h> // strrchr()/wcsrchr()
#include <stdlib.h> // malloc(), free()

#ifdef MMCS_WIN32
#include <Windows.h>
#include <shellapi.h> // CommandLineToArgvW()
#include <Objbase.h> // CoInitializeEx()
#include <winnt.h> // NtCurrentTeb()
#include <winternl.h> // PTEB, PPEB, NtCreateFile()
#include "win32_RegisterAsDefault.hpp"
#include "win32_MainWindow.hpp"
#include "msw_misc.hpp" // NT_MAX_PATH
#else
#include <unistd.h> // readlink()
#include <linux/limits.h> // PATH_MAX
#include <sys/types.h>
#include <sys/stat.h>
#endif

static const oschar * const portable_filename = _OS("mmcs_portable.txt");

static bool get_exe_and_dir(oschar ** exe_out, oschar ** dir_out)
{
	oschar * exe = NULL;
	oschar * dir = NULL;

#ifdef MMCS_WIN32
	exe = _wcsdup(
		NtCurrentTeb()->ProcessEnvironmentBlock->ProcessParameters->ImagePathName.Buffer
	);
	if (!exe) goto err;
#else
	// TODO whenever needed:
	// https://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe
	struct stat s;
	if (lstat("/proc/self/exe", &s) == -1) goto err;
	exe = (oschar *)malloc(s.st_size + 1);
	auto nbytes = readlink("/proc/self/exe", exe, s.st_size);
	if (nbytes == -1) goto err;
	exe[nbytes] = 0;
#endif

	oschar * rchr = osstrrchr(exe, _OS('/'));
#ifdef MMCS_WIN32
	oschar * bs = osstrrchr(exe, _OS('\\'));
	rchr = (rchr > bs) ? rchr : bs;
#endif
	if (!rchr) goto err; // nice
	auto nChars = rchr - exe;
	dir = (oschar *)malloc((nChars + 1) * sizeof(oschar));
	if (!dir) goto err; // double nice
	memcpy(dir, exe, nChars * sizeof(oschar));
	dir[nChars] = 0;
	*exe_out = exe;
	*dir_out = dir;
	return true;

err:
	free(exe);
	free(dir);
	return false;
}

static int main_inner(int argc, oschar ** argv)
{
	//#ifdef MMCS_WIN32
	//	UNICODE_STRING us;
	//	RtlInitUnicodeString(&us, L"mmcs_portable.txt");
	//	OBJECT_ATTRIBUTES objattr;
	//	InitializeObjectAttributes(
	//		&objattr,
	//		&us,
	//		OBJ_CASE_INSENSITIVE,
	//		curdir,
	//		NULL // security descriptor... use default
	//	);
	//	HANDLE hPortable;
	//	IO_STATUS_BLOCK statusblock;
	//	NTSTATUS ntstatus = NtCreateFile(
	//		&hPortable,
	//		FILE_GENERIC_READ,
	//		&objattr,
	//		&statusblock,
	//		0,
	//		FILE_ATTRIBUTE_NORMAL,
	//		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
	//		FILE_OPEN,
	//		0, // CreateOptions
	//		NULL, // EaBuffer
	//		0 // EaLength
	//	);
	//	if (!NT_SUCCESS(ntstatus)) {
	//		if (statusblock.Information != FILE_DOES_NOT_EXIST)
	//			return 1; // TODO: Log error?
	//		mmcs::isPortable = false;
	//	} else {
	//		mmcs::isPortable = true;
	//	}
	//#else
	//
	//#endif

#ifdef MMCS_WIN32
	mmcs::isPortable =
		GetFileAttributesW(portable_filename) != INVALID_FILE_ATTRIBUTES;
#else
	mmcs::isPortable =
		access(portable_filename, F_OK) != -1;
#endif

	if (mmcs::NativeMessaging_IsMode(argc, argv)) {
		// TODO: Figure out how portable will interact with this...
		return mmcs::NativeMessaging_Handler();
	}

#ifdef MMCS_WIN32
	return win32::MwEverything();
#else

#endif
}

int main(int argc, oschar ** argv)
{
	// global variables in commandline.hpp/cpp
	mmcs::argc = argc;
	mmcs::argv = argv;

	osfile curdir;

	oschar * exe;
	oschar * dir;
	if (!get_exe_and_dir(&exe, &dir))
		return 1; // TODO: log

	int ret = main_inner(argc, argv);

	// Cleanup...
	free(exe);
	free(dir);

	return ret;
}


#ifdef MMCS_WIN32
// Save the original current-directory then set the current-directory to the system-drive.
// This is done so a HANDLE to the original current-directory is not kept open.
// (HANDLE kept open = can't delete directory / eject drive)
// TODO: malloc the string for OriginalWorkingDirectory
//       nChars = GetCurrentDirectory(0, NULL);
//       malloc((nChars + 1) * sizeof(wchar_t));
//       etc...
bool reset_current_directory(void)
{
	DWORD nChars;
	wchar_t windows_dir[MAX_PATH]; // MAX_PATH (260) is intentional
	wchar_t system_drive[4] = {0, L':', L'\\', 0};
	nChars = GetCurrentDirectoryW(NT_MAX_PATH + 1, mmcs::OriginalWorkingDirectory);
	if (!nChars || nChars > NT_MAX_PATH + 1)
		return false; // TODO: Log?
	nChars = GetSystemWindowsDirectoryW(windows_dir, MAX_PATH);
	if (!nChars || nChars > NT_MAX_PATH + 1)
		return false; // TODO: Log?
	system_drive[0] = windows_dir[0];
	if (!SetCurrentDirectoryW(system_drive))
		return false; // TODO: Log?
	return true;
}

int CALLBACK wWinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow
)
{
	int argc = 1;
	wchar_t ** argv = NULL;
	static wchar_t * fake_argv[2] = {NULL, NULL};

	// Unused arguments.
	(void)hPrevInstance;
	(void)nCmdShow;

	//mmcs::hInstance = hInstance;

	if (!reset_current_directory()) return 1;

	// Skip needless allocations and directly check command-line
	if (wcscmp(lpCmdLine, REGISTER_AS_DEFAULT_ARG) == 0)
		return win32::RegisterAsDefault_Handler();

	// Avoid allocating since there's no args... little things add up...
	if (!*lpCmdLine) {
		argv = fake_argv;
		argv[0] = mmcs::ExePath;
	}
	else if (!(argv = CommandLineToArgvW(GetCommandLineW(), &argc)))
		return 1; // TODO: Log?

	// Some init
	// ShellExecute wants COM to be initialized...
	if (S_OK != CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))
		return 1; // TODO: Log?

	int ret = main(argc, argv);

	// Some uninit
	CoUninitialize(); // unnecessary but I'll keep it...

	return ret;
}
#endif
