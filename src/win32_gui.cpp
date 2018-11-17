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
#include "generated/win32_resource.h"
#include "win32_hinstance.h"
#include <commctrl.h> // InitCommonControlsEx()
#include "win32_ImagePainter.hpp"
#include "win32_MainWindow.hpp"
#include <Objbase.h> // CoInitializeEx()

namespace win32 {

HACCEL MainWindowAccelerators = NULL;
static HFONT DefaultMessageFont = NULL;
static HRESULT comInitStatus = S_FALSE;

// This only attempts to register classes once instead of on each MainWindow or ImagePainter creation.
// Also LoadIconW() and LoadCursorW() results can be shared...
// The downside is that class WndProcs need to be exported and pulled in from headers...
static bool registerClasses()
{
	HINSTANCE hInstance = HINST_THISCOMPONENT;
	HCURSOR cursorArrow = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
	HICON iconMMCS = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_MMCS_ICON));
	WNDCLASSEXW wc;

	if (!cursorArrow) return false;
	if (!iconMMCS) return false;

	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = win32::MainWindow::WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = iconMMCS;
	wc.hCursor = cursorArrow;
	wc.hbrBackground = (HBRUSH)(COLOR_GRAYTEXT + 1);
	wc.lpszMenuName = MAKEINTRESOURCEW(IDM_MAIN_MENU);
	wc.lpszClassName = L"MainWindow";
	wc.hIconSm = NULL; // If NULL, it will try to get a smaller icon from hIcon
	if (!RegisterClassExW(&wc))
		return false;

	wc.cbSize = sizeof(wc);
	wc.style = 0;
	wc.lpfnWndProc = win32::IpWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = cursorArrow;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"ImagePainter";
	wc.hIconSm = NULL;
	if (!RegisterClassExW(&wc))
		return false;

	return true;
}

static bool GuiInit_inner()
{	
	INITCOMMONCONTROLSEX controls;
	controls.dwSize = sizeof(controls);
	controls.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES | ICC_USEREX_CLASSES;
	if (!InitCommonControlsEx(&controls))
		return false; // TODO: Log error

	// ShellExecute wants COM to be initialized...
	// TODO: Windows::Foundation::Initialize() for Windows 10 to init instead?
	comInitStatus = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (comInitStatus != S_OK)
		return false;

	if (!registerClasses())
		return false;

	// Setup the system font to use...
	NONCLIENTMETRICSW ncMetrics;
	ncMetrics.cbSize = sizeof(ncMetrics);
	if (!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncMetrics), &ncMetrics, 0))
		return false;
	if (!(DefaultMessageFont = CreateFontIndirectW(&ncMetrics.lfMessageFont)))
		return false;

	if (!(MainWindowAccelerators = LoadAcceleratorsW(HINST_THISCOMPONENT, MAKEINTRESOURCEW(IDA_MAIN_WINDOW))))
		return false;

	return true;
}

bool GuiInit()
{
	if (!GuiInit_inner())
	{
		GuiUnInit();
		return false;
	}
	return true;
}

void GuiUnInit()
{
	if (comInitStatus == S_OK)
		CoUninitialize();
	if (DefaultMessageFont)
		(void)DeleteObject((HGDIOBJ)DefaultMessageFont);
}

// Use our system font please
void UseDefaultFont(HWND hwnd)
{
	(void)SendMessage(hwnd, WM_SETFONT, (WPARAM)DefaultMessageFont, TRUE);
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
