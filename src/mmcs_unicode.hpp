/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include <stddef.h> // size_t
#include <string>

// UTF16_CHAR_TYPE & UTF16_STRING_TYPE are used to work around a Visual Studio bug with std::u16string.
// unicode.obj : error LNK2001: unresolved external symbol "__declspec(dllimport) public: static class std::locale::id std::codecvt<char16_t,char,struct _Mbstatet>::id" (__imp_?id@?$codecvt@_SDU_Mbstatet@@@std@@2V0locale@2@A)
#ifdef _WIN32
#define UTF16_CHAR_TYPE wchar_t
#define UTF16_STRING_TYPE std::wstring
#else
#define UTF16_CHAR_TYPE char16_t
#define UTF16_STRING_TYPE std::u16string
#endif

namespace mmcs {

std::string utf16_to_utf8(const void * in, size_t len);
UTF16_STRING_TYPE utf8_to_utf16(const void * in, size_t len);

}
