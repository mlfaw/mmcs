/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_unicode.hpp"
#include <stddef.h> // size_t
#include <string> // string, u16string
#include <locale> // wstring_convert
#include <codecvt> // codecvt_utf8_utf16
#include <string.h> // strlen()

namespace mmcs {

static std::wstring_convert<std::codecvt_utf8_utf16<UTF16_CHAR_TYPE>, UTF16_CHAR_TYPE> converter;

std::string utf16_to_utf8(const void * in, size_t len)
{
	const UTF16_CHAR_TYPE * ws = (const UTF16_CHAR_TYPE *)in;
	if (len == 0) {
		auto tmp = ws;
		while (*tmp++) ++len; // wcslen for unix works on 4-byte wchar_t's
	}
	return converter.to_bytes(ws, ws + len);
}

UTF16_STRING_TYPE utf8_to_utf16(const void * in, size_t len)
{
	const char * str = (const char *)in;
	if (len == 0) len = strlen(str);
	return converter.from_bytes(str, str + len);
}

}
