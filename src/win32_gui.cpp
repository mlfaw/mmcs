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

// bool FuckYourFont(HWND hwnd)
// {
// 	// Fix shitty font stuff.
// 	static HFONT hFont = NULL;
// 	if (!hFont) {
// 		NONCLIENTMETRICSW ncMetrics;
// 		ncMetrics.cbSize = sizeof(ncMetrics);
// 		if (!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncMetrics), &ncMetrics, 0))
// 			return false;
// 		if (!(hFont = CreateFontIndirectW(&ncMetrics.lfMessageFont)))
// 			return false;
// 	}

// 	(void)SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, TRUE);
// 	return true;
// }

namespace win32 {

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
