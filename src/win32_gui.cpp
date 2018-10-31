/*{REPLACEMEWITHLICENSE}*/

// TODO: Read https://docs.microsoft.com/en-us/windows/desktop/hidpi/high-dpi-desktop-application-development-on-windows
// https://docs.microsoft.com/en-us/windows/desktop/hidpi/high-dpi-reference

// TODO: Remember _wsplitpath_s https://msdn.microsoft.com/en-us/library/8e46eyt7.aspx
// https://docs.microsoft.com/en-us/windows/desktop/Shutdown/wm-queryendsession
// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-shutdownblockreasoncreate
// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-senddlgitemmessagew
// http://www.winprog.org/tutorial/app_three.html -- toolbars
// http://www.winprog.org/tutorial/app_four.html  -- multiple child windows in a client area (mdi?)
// https://docs.microsoft.com/en-us/windows/desktop/api/intshcut/nf-intshcut-inetisoffline
// https://docs.microsoft.com/en-us/windows/desktop/api/WinBase/nf-winbase-readdirectorychangesw

#include "win32_gui.hpp"
#include <gdiplus.h>

namespace win32 {

static Gdiplus::GdiplusStartupInput gdiplusStartupInput;
static ULONG_PTR gdiplusToken = 0;
static HFONT hMessageFont = NULL;
static LOGFONTW messageFont;

static bool GuiInit_inner()
{
	// Setup the system font to use...
	NONCLIENTMETRICSW ncMetrics;
	ncMetrics.cbSize = sizeof(ncMetrics);
	if (!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncMetrics), &ncMetrics, 0))
		return false;
	messageFont = ncMetrics.lfMessageFont;

	auto status = Gdiplus::GdiplusStartup(
		&gdiplusToken,
		&gdiplusStartupInput,
		NULL
	);
	if (status != Gdiplus::Ok)
		return false;

	return true;
}

bool GuiInit()
{
	if (GuiInit_inner())
		return true;
	GuiUnInit();
	return false;
}

void GuiUnInit()
{
	if (gdiplusToken)
		Gdiplus::GdiplusShutdown(gdiplusToken);
}

// Use our system font please
void UseDefaultFont(HWND hwnd)
{
	HFONT hMessageFont = CreateFontIndirectW(&messageFont);
	if (hMessageFont)
		(void)SendMessage(hwnd, WM_SETFONT, (WPARAM)hMessageFont, TRUE);
}

static BOOL CALLBACK setFontCallback(HWND hwnd, LPARAM lParam)
{
	UseDefaultFont(hwnd);
	return TRUE;
}

// https://www.youtube.com/watch?v=73gGwGI8Z7E
void UseDefaultFontWithChildren(HWND hwnd)
{
	UseDefaultFont(hwnd);
	(void)EnumChildWindows(hwnd, setFontCallback, NULL);
}

int RunMessageLoop(pDoDispatch cb, void * user_data)
{
	MSG msg;
	BOOL bRet;
	while ((bRet = GetMessageW(&msg, NULL, 0, 0)) != 0)
	{
		if (bRet == -1)
			return -1;

		if (!cb || cb(&msg, user_data))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	return (int)msg.wParam;
}

// WM_GETMINMAXINFO
// limit the minimum and maximum size.
LRESULT MinMaxXandYonWindow(
	LPARAM lParam,
	LONG min_x,
	LONG min_y,
	LONG max_x,
	LONG max_y
)
{
	MINMAXINFO* mmi = (MINMAXINFO*)lParam;
	mmi->ptMinTrackSize.x = min_x;
	mmi->ptMinTrackSize.y = min_y;
	mmi->ptMaxTrackSize.x = max_x;
	mmi->ptMaxTrackSize.y = max_y;
	return 0;
}

// WM_NCHITTEST
// disable window borders from being user-resizable or changing the cursor
LRESULT DisableResizableWindowBorders(
	HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
)
{
	LRESULT ret = DefWindowProcW(hwnd, msg, wParam, lParam);
	switch (ret)
	{
	case HTBOTTOM:
	case HTBOTTOMLEFT:
	case HTBOTTOMRIGHT:
	case HTLEFT:
	case HTRIGHT:
	case HTTOP:
	case HTTOPLEFT:
	case HTTOPRIGHT:
		return HTCLIENT;
	}
	return ret;
}

}
