/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_sleep.h"

#ifdef _WIN32
#include <Windows.h> // Sleep()
#else
#include <time.h> // nanosleep()
#include <errno.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

void mmcs_sleepMS(unsigned int milliseconds)
{
#ifdef _WIN32
	Sleep(milliseconds);
#else
	struct timespec req, rem;
	time_t seconds = milliseconds / 1000;
	req.tv_sec = seconds;
	req.tv_nsec = ((long)milliseconds - seconds * 1000) * 1000 * 1000;
	while (nanosleep(&req, &rem) && errno == EINTR)
		req = rem;
#endif
}

inline void mmcs_artificallySlowMS(unsigned int milliseconds)
{
#if MMCS_ARTICIALLY_SLOW
	sleepMS(milliseconds);
#endif
}

#ifdef __cplusplus
}
#endif
