/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_Open.hpp"
#ifdef _WIN32
#include <Windows.h> // ShellExecuteA(), ShellExecuteW()
#elif MMCS_LINUX
#include <unistd.h> // fork(), execvp()
#include <stdlib.h> // exit(), free()
#endif

// TODO: Move ShellExecute functions to be ran in a helper process?
// Because ShellExecute caches verb-handlers and default file-type openers,
// which is a reasonable thing to do, but I'll leave future self to think it over.
// Also the RegisterAsDefault window is launched by a
// shellexecuteex(runas)... so don't forget that one too.

// TODO: Does ShellExecute reallyyyy need COM to be initialized? If so... COM might need to be initialized in every function for the thread-pool...

namespace mmcs {

#ifdef _WIN32
void CALLBACK worker_OpenUrl_NoFree(PTP_CALLBACK_INSTANCE instance, PVOID pv)
{
	OpenUrlSync((const char *)pv);
}
void CALLBACK worker_OpenUrl_FreeAfter(PTP_CALLBACK_INSTANCE instance, PVOID pv)
{
	OpenUrlSync((const char *)pv);
	free(pv);
}

void CALLBACK worker_OpenFile_NoFree(PTP_CALLBACK_INSTANCE instance, PVOID pv)
{
	OpenFileSync((const oschar *)pv);
}
void CALLBACK worker_OpenFile_FreeAfter(PTP_CALLBACK_INSTANCE instance, PVOID pv)
{
	OpenFileSync((const oschar *)pv);
	free(pv);
}
#endif

void OpenUrlSync(const char * url)
{
#ifdef _WIN32
	(void)ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
#elif MMCS_LINUX
	const char * xdg_open = "xdg-open";
	const char * argv[] = {xdg_open, url, NULL};
	if (fork() != 0) return; // if error or we're the parent just return
	execvp(xdg_open, argv);
	exit(1); // if we reach this then we've forked but execvp failed...
#endif
}

bool OpenUrlAsync(const char * url, bool FreeAfter)
{
#ifdef _WIN32
	PTP_SIMPLE_CALLBACK cb = FreeAfter ?
		  worker_OpenUrl_FreeAfter
		: worker_OpenUrl_NoFree;
	if (TrySubmitThreadpoolCallback(cb, (PVOID)url, NULL)) // NULL for process's default thread-pool
		return true;
#else

#endif
	return false;
}

void OpenFileSync(const oschar * file)
{
#ifdef _WIN32
	(void)ShellExecuteW(NULL, L"open", file, NULL, NULL, SW_SHOWNORMAL);
#elif MMCS_LINUX
	// xdg-open acts on both files and urls
	OpenUrlSync(file);
#endif
}

bool OpenFileAsync(const oschar * file, bool FreeAfter)
{
#ifdef _WIN32
	PTP_SIMPLE_CALLBACK cb = FreeAfter ?
		  worker_OpenFile_FreeAfter
		: worker_OpenFile_NoFree;
	if (TrySubmitThreadpoolCallback(cb, (PVOID)file, NULL)) // NULL for process's default thread-pool
		return true;
#else

#endif
	return false;
}

}
