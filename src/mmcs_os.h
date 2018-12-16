/*{REPLACEMEWITHLICENSE}*/
#pragma once

// oschar and such...
#ifdef _WIN32
#define oschar wchar_t
#define _OS(x) L ## x
#define osdirchar '\\'
#define osdirstr  "\\"
#define osstring  std::wstring
#define osstrlen  wcslen
#define osstrstr  wcsstr
#define osstrcmp  wcscmp
#define osstrrchr wcsrchr
#define osstrdup  _wcsdup
#define osstrcpy  wcscpy
#define osstrcat  wcscat
#else
#define oschar char
#define _OS(x) x
#define osdirchar '/'
#define osdirstr  "/"
#define osstring  std::string
#define osstrlen  strlen
#define osstrstr  strstr
#define osstrcmp  strcmp
#define osstrrchr strrchr
#define osstrdup  strdup
#define osstrcpy  strcpy
#define osstrcat  strcat
#endif

// file handle/fd type
#ifdef _WIN32
#define osfile HANDLE
#else
#define osfile int
#endif
