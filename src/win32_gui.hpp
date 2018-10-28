/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include <Windows.h>
namespace win32 {

bool GuiInit();
void GuiUnInit();

void UseDefaultFont(HWND hwnd);
void UseDefaultFontWithChildren(HWND hwnd);

typedef bool (*pDoDispatch)(MSG * msg, void * user_data);
int RunMessageLoop(pDoDispatch cb, void * user_data);

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
