/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_sleep.hpp"

#ifdef _WIN32
#include <Windows.h> // Sleep()
#else
#include <time.h> // nanosleep()
#include <errno.h>
#endif

namespace mmcs {

void sleepMS(unsigned int milliseconds)
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

inline void artificallySlowMS(unsigned int milliseconds)
{
#if MMCS_ARTICIALLY_SLOW
	sleepMS(milliseconds);
#endif
}

}

