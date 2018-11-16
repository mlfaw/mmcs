/*{REPLACEMEWITHLICENSE}*/
#pragma once

#ifndef MMCS_ARTIFICIALLY_SLOW
#define MMCS_ARTIFICIALLY_SLOW 0
#endif

namespace mmcs {

void sleepMS(unsigned int milliseconds);

inline void artificallySlowMS(unsigned int milliseconds);

}
