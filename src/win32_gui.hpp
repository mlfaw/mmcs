/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include <Windows.h>
namespace win32 {

extern HACCEL MainWindowAccelerators;

bool GuiInit();
void GuiUnInit();

void UseDefaultFont(HWND hwnd);
void UseDefaultFontWithChildren(HWND hwnd);

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
