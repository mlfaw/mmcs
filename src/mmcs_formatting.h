/*{REPLACEMEWITHLICENSE}*/
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

void mmcs_hexadecimal(const void * inbytes, char * str, int count, int uppercase);

#ifdef _WIN32
void mmcs_hexadecimalW(const void * inbytes, void * instr, int count, int uppercase);
#endif

#ifdef __cplusplus
}
#endif
