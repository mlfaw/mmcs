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
			mmcs::OpenUrlSync("https://mlfaw.com/mmcs");
			break;
		case IDM_HELP_GITHUB:
			mmcs::OpenUrlSync("https://github.com/mlfaw/mmcs");
			break;
		case IDM_HELP_CHANGELOG:
		{
			const oschar * const changelogtxt = _OS("/changelog.txt");
			auto len = osstrlen(mmcs::ExeDir) + osstrlen(changelogtxt) + 1;
			oschar * changelogpath = (oschar *)malloc(len * sizeof(oschar));
			if (!changelogpath)
				return;
			osstrcpy(changelogpath, mmcs::ExeDir);
			osstrcat(changelogpath, changelogtxt);
			mmcs::OpenFileSync(changelogpath);
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
	case NM_RCLICK:
	{
		// return non-zero to process ourselves instead of it passing up to MainWindow's WM_CONTEXTMENU.

		int idx = tabbar_.TabUnderMouse();
		if (idx == -1)
			return 1;

		POINT point;
		if (!GetCursorPos(&point))
			return 1;
	
		tabbar_.right_click_idx_ = idx;
		BOOL ret = TrackPopupMenu(
			tabbar_.context_menu_,
			TPM_RETURNCMD, // flags
			point.x,
			point.y,
			0, // reserved
			hwnd,
			NULL // prcRect
		);

		if (!ret) tabbar_.right_click_idx_ = -1;

		return 1;
	}
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

void MainWindow::WmSize(HWND hwnd, UINT state, int cx, int cy)
{
	// TODO: Call tabbar_->OnSize(hdwp, cx, cy);
	//       Inside tabbar_->OnSize(), call tabpage_->OnSize(hdwp, ax, ay);

	HDWP hdwp = BeginDeferWindowPos(5);

	if (!(hdwp = tabbar_.DeferSize(hdwp, cx, cy)))
		return;

	(void)EndDeferWindowPos(hdwp);
}

void MainWindow::WmMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys)
{
	fitted_ = !fitted_;
	if (fitted_)
		return;



	RECT rc;
	GetClientRect(tabbar_.hwnd_, &rc);
	TabCtrl_AdjustRect(tabbar_.hwnd_, FALSE, &rc);

	if (1)
		MessageBoxA(NULL, "test", "test", 0);

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
		HANDLE_MSG(hwnd, WM_SIZE,       mw->WmSize);
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

	win32::UseDefaultFontWithChildren(hwnd_);

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
