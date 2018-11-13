/*{REPLACEMEWITHLICENSE}*/

// https://curl.haxx.se/libcurl/c/libcurl-tutorial.html
// TODO: CURLOPT_VERBOSE, CURLOPT_DEBUGFUNCTION, CURLOPT_FORBID_REUSE?
// TODO: curl_global_init_mem() with strdup() checking a hash-table for static-strings... or something?
// TODO: Remove CURL and wrap libtls around an HTTP header parser?
//       libtls has tls_config_set_ca_mem() which is :ok_hand:

#if MMCS_WIN32
// This motherfucker is right here at the top before anything can pull in Windows.h
#include <WinSock2.h>
#endif

#include "mmcs_os.hpp"
#include "mmcs_globals.hpp"
#include "mmcs_file.hpp"
#include "mmcs_NativeMessaging.hpp"
#include <string.h> // strrchr()/wcsrchr()
#include <stdlib.h> // malloc(), free()
#include <stddef.h> // ptrdiff_t

#if MMCS_WIN32
#include <Windows.h>
#include <shellapi.h> // CommandLineToArgvW()
#include <Objbase.h> // CoInitializeEx()
#include <winnt.h> // NtCurrentTeb()
#include <winternl.h> // PTEB, PPEB
#include "win32_RegisterAsDefault.hpp"
#include "win32_MainWindow.hpp"
#include "win32_gui.hpp"
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
	oschar * rchr;
	ptrdiff_t nChars;

#if MMCS_WIN32
	oschar * bs; // backslash

	exe = _wcsdup(
		NtCurrentTeb()->ProcessEnvironmentBlock->ProcessParameters->ImagePathName.Buffer
	);
	if (!exe) goto err;
#else
	ssize_t nbytes;
	// TODO whenever needed:
	// https://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe
	struct stat s;
	ssize_t lsize;
	if (lstat("/proc/self/exe", &s) == -1) return false;
	lsize = s.st_size ? s.st_size : PATH_MAX;
	exe = (oschar *)malloc(lsize + 1);
	if (!exe) return false;
	nbytes = readlink("/proc/self/exe", exe, lsize);
	if (nbytes == -1) goto err;
	exe[nbytes] = 0;
	if (s.st_size == 0) { // resize buffer from PATH_MAX
		char * x = (char *)realloc(exe, nbytes+1);
		if (!x) goto err;
		exe = x;
	}
#endif

	rchr = osstrrchr(exe, _OS('/'));
#if MMCS_WIN32
	bs = osstrrchr(exe, _OS('\\'));
	rchr = (rchr > bs) ? rchr : bs;
#endif
	if (!rchr) goto err; // nice
	nChars = rchr - exe;
	dir = (oschar *)malloc((nChars + 1) * sizeof(oschar));
	if (!dir) goto err; // double nice
	memcpy(dir, exe, nChars * sizeof(oschar));
	dir[nChars] = 0;
	*exe_out = exe;
	*dir_out = dir;
	return true;

err:
	free(exe);
	return false;
}

static int main_inner(int argc, oschar ** argv)
{
	osfile hExeDir = mmcs::file::simpleOpen(mmcs::ExeDir, "rdP");
	if (hExeDir != (osfile)-1) {
		osfile hPortable = mmcs::file::simpleRelativeOpen(hExeDir, _OS("mmcs_portable.txt"), "rM");
		if (hPortable == (osfile)-1) {
			mmcs::isPortable = false;
		} else {
			mmcs::isPortable = true;
			mmcs::file::closeFile(hPortable);
		}
		mmcs::file::closeFile(hExeDir);
	} else {
#if MMCS_WIN32
		return 1; // TODO: this ain't good...
#else
		// TODO: is setting portable to false okay?
		mmcs::isPortable = false;
#endif
	}

	if (mmcs::NativeMessaging_IsMode(argc, argv)) {
		// TODO: Figure out how portable will interact with this...
		return mmcs::NativeMessaging_Handler();
	}

#if MMCS_WIN32
	if (!win32::GuiInit())
		return 1;

	win32::MainWindow mw;
	if (!mw.Init(600, 400, CW_USEDEFAULT, CW_USEDEFAULT, false))
		return 1;
	int ret = mw.Run();

	win32::GuiUnInit();

	// TODO: cleanly shutdown any tasks (io, db, network, etc)
	// just exit for now though

	return ret;
#else
	return 0;
#endif
}

#if 0
#include <tls.h>
#define CURL_STATICLIB
#include <curl/curl.h>
#endif

int main(int argc, oschar ** argv)
{
	// global variables in commandline.hpp/cpp
	mmcs::argc = argc;
	mmcs::argv = argv;

	//osfile curdir;

	if (!get_exe_and_dir(&mmcs::ExePath, &mmcs::ExeDir))
		return 1; // TODO: log

	int ret = main_inner(argc, argv);

	// Cleanup...
	free(mmcs::ExePath);
	free(mmcs::ExeDir);

	return ret;
}


#if MMCS_WIN32
// Save the original current-directory then set the current-directory to the system-drive.
// This is done so a HANDLE to the original current-directory is not kept open.
// (HANDLE kept open = can't delete directory / eject drive)
// TODO: malloc the string for OriginalWorkingDirectory
//       nChars = GetCurrentDirectory(0, NULL);
//       malloc((nChars + 1) * sizeof(wchar_t));
//       etc...
bool reset_current_directory()
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

	if (argv != fake_argv)
		(void)LocalFree((HLOCAL)argv);

	return ret;
}
#endif
