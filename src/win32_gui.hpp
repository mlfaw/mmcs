/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include <Windows.h>
//#include "generated/win32_resource.h"

namespace win32 {

enum WINDOW_MESSAGES {
	WM_MY_NOTHING = WM_APP + 50,
	WM_MY_OPEN_FILES,
};

// WM_GETMINMAXINFO
// limit the minimum and maximum size.
LRESULT MinMaxXandYonWindow(
	LPARAM lParam,
	LONG min_x,
	LONG min_y,
	LONG max_x,
	LONG max_y
);

// WM_NCHITTEST
// disable window borders from being user-resizable or changing the cursor
LRESULT DisableResizableWindowBorders(
	HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
);

}
