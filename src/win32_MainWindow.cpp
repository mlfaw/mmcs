/*{REPLACEMEWITHLICENSE}*/
#include "win32_MainWindow.hpp"
#include <Windows.h>
#include <commctrl.h> // Staturs bar control
#include <shellapi.h> // DragAcceptFiles(), DragFinish(), DragQueryPoint(), DragQueryFileW()
// "Message crackers" and other helpful functions
// https://docs.microsoft.com/en-us/windows/desktop/api/windowsx/
#include <windowsx.h>

#include <vector>
#include <string>

#include "generated/win32_resource.h"
#include "win32_RegisterAsDefault.hpp"
#include "mmcs_Open.hpp" // OpenUrlAsync(), OpenFileAsync()
#include "mmcs_os.hpp"
#include "mmcs_GetDirectoryFiles.hpp"
#include "mmcs_globals.hpp"
#include "win32_gui.hpp"
#include "win32_hinstance.h"
#include "mmcs_SelectFilesWindow.hpp"
#include "win32_ImagePainter.hpp"

#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>

#include <gdiplus.h>

namespace win32 {

void MainWindow::WmCommand(HWND hwnd, int id, HWND hwndCtrl, UINT codeNotify)
{
	// Menu or Accelerator
	if (hwndCtrl == (HWND)0 || hwndCtrl == (HWND)1)
	{
		switch (id)
		{
		case 0: // User clicked on a separator...
			break;
		case IDM_FILE_NEW_TAB:
		{
			//static int fuck = 1;
			//wchar_t buf[20] = {};
			//swprintf(buf, L"files (%d)", ++fuck);
			tabbar_.Append(L"files (0)");
			break;
		}
		case IDM_CLOSE_TAB:
		{
			int idx = tabbar_.right_click_idx_;
			if (idx == -1)
				// Coming from Ctrl+W if invalid right_click_idx_
				if (-1 == (idx = TabCtrl_GetCurSel(tabbar_.hwnd_)))
					break;
			tabbar_.right_click_idx_ = -1;
			tabbar_.Remove(idx);
			break;
		}
		case IDM_FILE_OPEN_FILES:
		case IDM_FILE_OPEN_DIRS:
		{
			std::vector<osstring> * results;
			bool foldersOnly = id == IDM_FILE_OPEN_DIRS;
			if (mmcs::SelectFilesWindow(&results, foldersOnly, true))
			{
				for (const auto & x : *results)
				{
					MessageBoxW(NULL, x.c_str(), L"", MB_OK);
				}
				delete results;
			}
			break;
		}
		case IDM_FILE_EXIT:
			(void)DestroyWindow(hwnd);
			break;
		case IDM_VIEW_TOGGLE_DARK_MODE:
		{
			HMENU hMenu = GetMenu(hwnd);
			MENUITEMINFOW info;
			info.cbSize = sizeof(info);
			info.fMask = MIIM_STATE;
			if (!GetMenuItemInfoW(hMenu, IDM_VIEW_TOGGLE_DARK_MODE, FALSE, &info))
				return;
			if (info.fState & MFS_CHECKED)
				info.fState &= ~MFS_CHECKED;
			else
				info.fState |= MFS_CHECKED;
			if (!SetMenuItemInfoW(hMenu, IDM_VIEW_TOGGLE_DARK_MODE, FALSE, &info))
				return;
			(void)DrawMenuBar(hwnd);
			break;
		}
		case IDM_HELP_WEBSITE:
			mmcs::OpenUrlAsync("https://mlfaw.com/mmcs", false);
			break;
		case IDM_HELP_GITHUB:
			mmcs::OpenUrlAsync("https://github.com/mlfaw/mmcs", false);
			break;
		case IDM_HELP_CHANGELOG:
		{
			const oschar changelogtxt[] = _OS("/changelog.txt");
			auto len = osstrlen(mmcs::ExeDir) + sizeof(changelogtxt);
			oschar * changelogpath = (oschar *)malloc(len * sizeof(oschar));
			if (!changelogpath)
				return;
			osstrcpy(changelogpath, mmcs::ExeDir);
			osstrcat(changelogpath, changelogtxt);
			if (!mmcs::OpenFileAsync(changelogpath, true))
				free(changelogpath);
			break;
		}
		case IDM_HELP_REGISTER_AS_DEFAULT:
			win32::RegisterAsDefault_Launch();
			break;
		case IDM_HELP_ABOUT:
			break;
		}
	}
	else // Control
	{

	}
}

LRESULT MainWindow::WmNotify(HWND hwnd, int ctrlId, NMHDR * info)
{
	switch (info->code)
	{
	case TCN_SELCHANGING:
		return FALSE; // FALSE to allow changing
	case TCN_SELCHANGE:
		return 0; // return value doesn't matter
	}

	return 0;
}

BOOL MainWindow::WmCreate(HWND hwnd, LPCREATESTRUCT cs)
{
	//if (!win32::ImagePainter_Create(hwnd)) return FALSE;
	if (!tabbar_.Init(hwnd))
		return FALSE;
	return TRUE;
}

void MainWindow::WmClose(HWND hwnd)
{
	WINDOWPLACEMENT wp = {};
	wp.length = sizeof(wp);
	GetWindowPlacement(hwnd, &wp);
	// TODO: serialize & save wp ; SetWindowPos(&wp) to use last position...

	(void)DestroyWindow(hwnd);
}

// NOTE: This handles all drop-files from child windows.
// A child window can handle drop-files that target them if they
// have the extended window style WS_EX_ACCEPTFILES
// or it has been enabled through DragAcceptFiles().
void MainWindow::WmDropFiles(HWND hwnd, HDROP hdrop)
{
	POINT dropPoint;
	BOOL droppedInClientArea = DragQueryPoint(hdrop, &dropPoint);
	UINT fileCount = DragQueryFileW(hdrop, 0xFFFFFFFF, NULL, 0);

	DragFinish(hdrop);
}

void MainWindow::WmShowWindow(HWND hwnd, BOOL fShow, UINT status)
{

}

void MainWindow::WmEndSession(HWND hwnd, BOOL fEnding)
{
	PostQuitMessage(0);
}

void MainWindow::WmDestroy(HWND hwnd)
{
	PostQuitMessage(0);
}

static inline void imageFit(
	Gdiplus::RectF & destRect,
	Gdiplus::RectF & imgRect,
	double imgW,
	double imgH,
	double clW,
	double clH
)
{
	double destX, destY, destW, destH;
	double scaleH = clH / imgH;
	double scaleW = clW / imgW;

	if (scaleW < scaleH)
	{
		destW = clW;
		destH = imgH * scaleW;
		destX = 0;
		destY = (clH - destH) / 2;
	}
	else
	{
		destW = imgW * scaleH;
		destH = clH;
		destX = (clW - destW) / 2;
		destY = 0;
	}

	destRect = Gdiplus::RectF(
		(Gdiplus::REAL)destX,
		(Gdiplus::REAL)destY,
		(Gdiplus::REAL)destW,
		(Gdiplus::REAL)destH
	);
}

void MainWindow::GetImageLocation(
	Gdiplus::RectF & destRect,
	Gdiplus::RectF & imgRect,
	double imgW,
	double imgH,
	double clW,
	double clH
)
{
	if (fitted_)
	{
		imageFit(
			destRect,
			imgRect,
			imgW,
			imgH,
			clW,
			clH
		);
		return;
	}


}

void MainWindow::WmPaint(HWND hwnd)
{
	if (!image_)
	{
		image_ = Gdiplus::Image::FromFile(L"C:\\code\\mmcs\\900kb.jpg", FALSE);
	}

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	if (!hdc) return;

	if (image_)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);

		Gdiplus::RectF destRect;
		Gdiplus::RectF imgRect;
		Gdiplus::Unit imgUnit;
		image_->GetBounds(&imgRect, &imgUnit);

		GetImageLocation(
			destRect,
			imgRect,
			imgRect.Width,
			imgRect.Height,
			(double)rc.right,
			(double)rc.bottom
		);

		auto pGraphics = Gdiplus::Graphics::FromHDC(hdc);

		pGraphics->DrawImage(
			image_,
			destRect,
			imgRect,
			imgUnit,
			(const Gdiplus::ImageAttributes *)NULL
		);
	
		delete pGraphics;
	}

	(void)EndPaint(hwnd, &ps);
}

#ifndef SWP_STATECHANGED
#define SWP_STATECHANGED 0x8000
#endif
#ifndef SWP_NOCLIENTSIZE
#define SWP_NOCLIENTSIZE 0x0800
#endif
#ifndef SWP_NOCLIENTMOVE
#define SWP_NOCLIENTMOVE 0x1000
#endif

void MainWindow::WmWindowPosChanged(HWND hwnd, const LPWINDOWPOS pos)
{
	LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
	if (!style) return;

	if ((pos->flags & SWP_STATECHANGED) || !(pos->flags & SWP_NOCLIENTSIZE))
	//if (!(pos->flags & SWP_NOSIZE))
	{
		RECT rc;
		if (!GetClientRect(hwnd, &rc)) return;

		UINT state;
		if (style & WS_MINIMIZE)
			state = SIZE_MINIMIZED;
		else if (style & WS_MAXIMIZE)
			state = SIZE_MAXIMIZED;
		else
			state = SIZE_RESTORED;

		WmSize(hwnd, state, rc.right, rc.bottom);
	}
}

// TODO: 
void MainWindow::WmSize(HWND hwnd, UINT state, int cx, int cy)
{
	// TODO: Call tabbar_->OnSize(hdwp, cx, cy);
	//       Inside tabbar_->OnSize(), call tabpage_->OnSize(hdwp, ax, ay);

	HDWP hdwp = BeginDeferWindowPos(5);
	if (!hdwp) return;

	hdwp = DeferWindowPos(
		hdwp,
		tabbar_.hwnd_,
		NULL, // hWndInsertAfter
		0, // x
		0, // y
		cx,
		cy,
		SWP_NOZORDER
	);

	if (!hdwp) return;

	HWND tabpage = GetDlgItem(hwnd_, ID_TAB_PAGE);
	if (tabpage)
	{
		RECT rc2;
		rc2.left = 0;
		rc2.top = 0;
		rc2.right = cx;
		rc2.bottom = cy;
		TabCtrl_AdjustRect(tabbar_.hwnd_, FALSE, &rc2);

		hdwp = DeferWindowPos(
			hdwp,
			tabpage,
			NULL, // hWndInsertAfter
			rc2.left, // x
			rc2.top, // y
			rc2.right - rc2.left,
			rc2.bottom - rc2.top,
			SWP_NOZORDER
		);

		if (!hdwp) return;
	}

	(void)EndDeferWindowPos(hdwp);
}

void MainWindow::WmMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys)
{
	fitted_ = !fitted_;
	if (fitted_)
		return;


	return;
}

LRESULT CALLBACK MainWindow::WndProc(
	HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
)
{
	if (msg == WM_NCCREATE)
	{
		SetLastError(0);
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)(((LPCREATESTRUCTW)lParam)->lpCreateParams));
		if (GetLastError())
			return FALSE;
		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}

	auto mw = (MainWindow *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

	switch (msg)
	{
		HANDLE_MSG(hwnd, WM_COMMAND,    mw->WmCommand);
		HANDLE_MSG(hwnd, WM_CREATE,     mw->WmCreate);
		HANDLE_MSG(hwnd, WM_CLOSE,      mw->WmClose);
		HANDLE_MSG(hwnd, WM_DROPFILES,  mw->WmDropFiles);
		HANDLE_MSG(hwnd, WM_SHOWWINDOW, mw->WmShowWindow);
		HANDLE_MSG(hwnd, WM_ENDSESSION, mw->WmEndSession);
		HANDLE_MSG(hwnd, WM_DESTROY,    mw->WmDestroy);
		//HANDLE_MSG(hwnd, WM_PAINT,      mw->WmPaint);
		HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGED, mw->WmWindowPosChanged);
		//HANDLE_MSG(hwnd, WM_SIZE,       mw->WmSize); // handled in WmWindowPosChanged()
		HANDLE_MSG(hwnd, WM_MOUSEWHEEL, mw->WmMouseWheel);
		HANDLE_MSG(hwnd, WM_NOTIFY,     mw->WmNotify);
	}

	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool MainWindow::DoDispatchCheck(MSG * msg, void * user_data)
{
	auto mw = (MainWindow *)user_data;
	return !TranslateAcceleratorW(mw->hwnd_, mw->accel_, msg);
}

int MainWindow::Run()
{
	return win32::RunMessageLoop(MainWindow::DoDispatchCheck, (void *)this);
}

HACCEL MainWindow::CreateAccelerators()
{
	//typedef struct tagACCEL {
	//    BYTE   fVirt; /* Also called the flags field */
	//    WORD   key;
	//    WORD   cmd;
	//} ACCEL, *LPACCEL;

	// https://docs.microsoft.com/en-us/windows/desktop/inputdev/virtual-key-codes

	ACCEL accels[] = {
		{ // Ctrl+t = New Tab
			FCONTROL | FVIRTKEY,
			0x54, // T
			IDM_FILE_NEW_TAB
		},
		{ // Ctrl+w = Close Tab
			FCONTROL | FVIRTKEY,
			0x57, // W
			IDM_CLOSE_TAB
		},
	};

	return CreateAcceleratorTableW(accels, sizeof(accels) / sizeof(accels[0]));
}

bool MainWindow::Init(int w, int h, int x, int y, bool maximize)
{
	INITCOMMONCONTROLSEX controls;
	controls.dwSize = sizeof(controls);
	controls.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES | ICC_USEREX_CLASSES;
	if (!InitCommonControlsEx(&controls))
		return false; // TODO: Log error

	HINSTANCE hInstance = HINST_THISCOMPONENT;//mmcs::hInstance;//(HINSTANCE)GetModuleHandleW(NULL);

	HICON iconMMCS = (HICON)LoadImageW(
		hInstance,
		MAKEINTRESOURCEW(IDI_MMCS_ICON),
		IMAGE_ICON,
		0,
		0,
		LR_SHARED
	);

	HCURSOR cursorArrow = (HCURSOR)LoadImageW(
		NULL,
		(LPCWSTR)IDC_ARROW,
		IMAGE_CURSOR,
		0,
		0,
		LR_SHARED
	);

	WNDCLASSEXW wc;
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWindow::WndProc;
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

	if (!(accel_ = CreateAccelerators()))
		return false;

	hwnd_ = CreateWindowExW(
		WS_EX_ACCEPTFILES,
		L"MainWindow",
		L"MMCS by mlfaw",
		WS_OVERLAPPEDWINDOW,
		x,
		y,
		w,
		h,
		NULL, // hwndParent
		NULL, // hMenu (NULL if using class-menu)
		hInstance,
		this // lpParam
	);

	if (!hwnd_)
		goto err;

	//win32::UseDefaultFontWithChildren(hwnd_);

	(void)ShowWindow(hwnd_, maximize ? SW_SHOWMAXIMIZED : SW_SHOW);
	(void)UpdateWindow(hwnd_);
	return true;

err:
	if (accel_)
		(void)DestroyAcceleratorTable(accel_);
	if (hwnd_)
		DestroyWindow(hwnd_);
	return false;
}

}
