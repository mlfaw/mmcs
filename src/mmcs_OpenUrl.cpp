/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_OpenUrl.hpp"

#ifdef MMCS_WIN32
#include <Windows.h>
#include <shellapi.h> // ShellExecuteA()
#elif defined(unix) || defined(__unix__) || defined(__unix)
#include <unistd.h> // fork(), execvp()
#include <stdlib.h> // exit()
#endif

namespace mmcs {

#ifdef MMCS_WIN32
void OpenUrlSync(const char * url)
{
	(void)ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
}
#elif defined(unix) || defined(__unix__) || defined(__unix)
void OpenUrlSync(const char * url)
{
	const char * xdg_open = "xdg-open";
	const char * argv[] = {xdg_open, url, NULL};
	if (fork() != 0) return; // if error or we're the parent just return
	execvp(xdg_open, argv);
	exit(1); // if we reach this then we've forked but execvp failed...
}
#endif

}
