/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_formatting.h"
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

void mmcs_hexadecimal(const void * inbytes, char * str, int count, int uppercase)
{
    const unsigned char * bytes = (const unsigned char *)inbytes;
    const char * fmt = uppercase ? "%02X" : "%02x";
    for (int i = 0; i < count; ++i) {
        sprintf(str, fmt, bytes[i]);
        str += 2;
    }
}

#ifdef _WIN32
void mmcs_hexadecimalW(const void * inbytes, void * instr, int count, int uppercase)
{
    wchar_t * str = (wchar_t *)instr;
    const unsigned char * bytes = (const unsigned char *)inbytes;
    const wchar_t * fmt = uppercase ? L"%02X" : L"%02x";
    for (int i = 0; i < count; ++i) {
        swprintf(str, 3, fmt, bytes[i]);
        str += 2;
    }
}
#endif


#ifdef __cplusplus
}
#endif
