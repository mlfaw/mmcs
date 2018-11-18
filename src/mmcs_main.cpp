/*{REPLACEMEWITHLICENSE}*/

// https://curl.haxx.se/libcurl/c/libcurl-tutorial.html
// TODO: CURLOPT_VERBOSE, CURLOPT_DEBUGFUNCTION, CURLOPT_FORBID_REUSE?
// TODO: curl_global_init_mem() with strdup() checking a hash-table for static-strings... or something?
// TODO: Remove CURL and wrap libtls around an HTTP header parser?
//       With libtls, tls_config_set_ca_mem() can be used to remove libressl/openssl's usage of a hardcoded _PATH_SSL_CA_FILE

#include "mmcs_os.hpp"
#include "mmcs_globals.hpp"
#include "mmcs_file.hpp"
#include "mmcs_NativeMessaging.hpp"
#include "mmcs_ScopedFree.hpp"
#include <string.h> // strrchr()/wcsrchr()
#include <stdlib.h> // malloc(), free()

#ifdef _WIN32
#include <Windows.h>
#include <shellapi.h> // CommandLineToArgvW()
#include "win32_RegisterAsDefault.hpp"
#include "win32_MainWindow.hpp"
#include "win32_gui.hpp"
#include "win32_misc.hpp" // win32::GetExePath()
#else
#include <unistd.h> // readlink()
#include <linux/limits.h> // PATH_MAX
#include <sys/types.h>
#include <sys/stat.h>
#endif

static const oschar portable_filename[] = _OS("mmcs_portable.txt");

static oschar * getExeDir()
{
#ifdef _WIN32
	return mmcs::file::getDir(win32::GetExePath());
#elif defined(__linux__)
	const char proc_exe[] = "/proc/self/exe";
	struct stat s;
	if (lstat(proc_exe, &s) == -1) return NULL;
	ssize_t lsize = s.st_size ? s.st_size : PATH_MAX;
	char * exe = (char *)malloc(lsize + 1);
	if (!exe) return NULL;
	ssize_t nbytes = readlink(proc_exe, exe, lsize);
	if (nbytes == -1) {
		free(exe);
		return NULL;
	}
	exe[nbytes] = 0;
	if (s.st_size == 0) { // resize buffer from PATH_MAX
		char * x = (char *)realloc(exe, nbytes+1);
		if (x) exe = x;
	}

	char * dir = mmcs::file::getDir(exe);
	free(exe);
	return dir;
#endif

	return NULL;
}

int main(int argc, oschar ** argv)
{
	if (!(mmcs::ExeDir = getExeDir()))
		return 1;
	mmcs::ScopedFree sf_ExeDir(mmcs::ExeDir);

	osfile hExeDir = mmcs::file::simpleOpen(mmcs::ExeDir, "rdP");
	if (hExeDir != (osfile)-1) {
		osfile hPortable = mmcs::file::simpleRelativeOpen(hExeDir, portable_filename, "rM");
		if (hPortable == (osfile)-1) {
			mmcs::isPortable = false;
		} else {
			mmcs::isPortable = true;
			mmcs::file::closeFile(hPortable);
		}
		mmcs::file::closeFile(hExeDir);
	} else {
#ifdef _WIN32
		return 1; // TODO: this ain't good...
#else
		// TODO: is setting portable to false okay?
		mmcs::isPortable = false;
#endif
	}

	if (mmcs::NativeMessaging::Handler(argc, argv))
		return 0;

#ifdef _WIN32
	if (!win32::GuiInit())
		return 1;

	win32::MainWindow mw;
	if (!mw.Init(1200, 900, CW_USEDEFAULT, CW_USEDEFAULT, false))
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

#ifdef _WIN32
// Save the original current-directory then set the current-directory to the system-drive.
// This is done so a HANDLE to the original current-directory is not kept open.
// (HANDLE kept open = can't delete directory / eject drive)
bool reset_current_directory()
{
	DWORD nChars;
	wchar_t windows_dir[MAX_PATH]; // MAX_PATH (260) is intentional
	wchar_t system_drive[4] = {0, L':', L'\\', 0};
	if (!(nChars = GetCurrentDirectoryW(0, NULL)))
		return false;
	if (!(mmcs::OriginalWorkingDirectory = (wchar_t *)malloc(nChars * sizeof(wchar_t))))
		return false;
	if (!GetCurrentDirectoryW(nChars, mmcs::OriginalWorkingDirectory))
		return false;
	nChars = GetSystemWindowsDirectoryW(windows_dir, MAX_PATH);
	if (!nChars || nChars > MAX_PATH)
		return false;
	system_drive[0] = windows_dir[0];
	if (!SetCurrentDirectoryW(system_drive))
		return false;
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
	static wchar_t * fake_argv[2] = {L"mmcs.exe", NULL};

	// Unused arguments.
	(void)hInstance;
	(void)hPrevInstance;
	(void)nCmdShow;

	if (!reset_current_directory()) return 1;

	if (win32::RegisterAsDefault::Handler(lpCmdLine))
		return 0;

	// Avoid allocating since there's no args... little things add up...
	if (!*lpCmdLine)
		argv = fake_argv;
	else if (!(argv = CommandLineToArgvW(GetCommandLineW(), &argc)))
		return 1; // TODO: Log?

	int ret = main(argc, argv);

	if (argv != fake_argv)
		(void)LocalFree((HLOCAL)argv);

	return ret;
}
#endif
