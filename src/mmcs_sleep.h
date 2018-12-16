/*{REPLACEMEWITHLICENSE}*/
#pragma once

#ifndef MMCS_ARTIFICIALLY_SLOW
#define MMCS_ARTIFICIALLY_SLOW 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

void mmcs_sleepMS(unsigned int milliseconds);

inline void mmcs_artificallySlowMS(unsigned int milliseconds);

#ifdef __cplusplus
}
#endif
