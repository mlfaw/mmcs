/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include <Windows.h>

namespace win32 {

LRESULT CALLBACK IpWindowProc(
	HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
);

bool ImagePainter_Create(HWND hParent);

}
