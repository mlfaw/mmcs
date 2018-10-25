/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_Open.hpp"
#if MMCS_WIN32
#include <Windows.h> // ShellExecuteA(), ShellExecuteW()
#elif MMCS_LINUX
#include <unistd.h> // fork(), execvp()
#include <stdlib.h> // exit()
#endif

namespace mmcs {

void OpenUrlSync(const char * url)
{
#if MMCS_WIN32
	(void)ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
#elif MMCS_LINUX
	const char * xdg_open = "xdg-open";
	const char * argv[] = {xdg_open, url, NULL};
	if (fork() != 0) return; // if error or we're the parent just return
	execvp(xdg_open, argv);
	exit(1); // if we reach this then we've forked but execvp failed...
#endif
}

void OpenFileSync(const oschar * file)
{
#if MMCS_WIN32
	(void)ShellExecuteW(NULL, L"open", file, NULL, NULL, SW_SHOWNORMAL);
#elif MMCS_LINUX
	// xdg-open acts on both files and urls
	OpenUrlSync(file);
#endif
}

}
