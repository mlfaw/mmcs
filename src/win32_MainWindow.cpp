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
			break;
		case IDM_FILE_OPEN_FILES:
			break;
		case IDM_FILE_OPEN_FOLDERS:
		{
			std::vector<osstring> * results;
			if (mmcs::SelectFilesWindow(&results, true, false)) {
				auto & x = results->at(0);
				auto y = x.c_str();
				SetCurrentDirectoryW(y);
				MessageBoxW(NULL, y, L"hey", MB_OK);
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

BOOL MainWindow::WmCreate(HWND hwnd, LPCREATESTRUCT cs)
{
	//if (!win32::ImagePainter_Create(hwnd)) return FALSE;
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

Gdiplus::Image * Image = NULL;
void MainWindow::WmPaint(HWND hwnd)
{
	if (!Image)
	{
		Image = Gdiplus::Image::FromFile(L"C:\\code\\mmcs\\Dk92uSaX0AAj3yT.png", FALSE);
	}

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	if (!hdc) return;

	if (Image)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);

		Gdiplus::RectF imgRect;
		Gdiplus::Unit imgUnit;
		Image->GetBounds(&imgRect, &imgUnit);

		double imgW = imgRect.Width;
		double imgH = imgRect.Height;
		double clW = (double)rc.right;
		double clH = (double)rc.bottom;
		double outX = 0, outY = 0, outW = clW, outH = clH;
		double scaleH = clH / imgH,
			scaleW = clW / imgW;

		if (scaleW < scaleH)
		{
			outW = clW;
			outH = imgH * scaleW;
			outX = 0;
			outY = (clH - outH) / 2;
		}
		else
		{
			outW = imgW * scaleH;
			outH = clH;
			outX = (clW - outW) / 2;
			outY = 0;
		}

		auto destRect =  Gdiplus::RectF(
			(Gdiplus::REAL)outX,
			(Gdiplus::REAL)outY,
			(Gdiplus::REAL)outW,
			(Gdiplus::REAL)outH
		);

		auto pGraphics = Gdiplus::Graphics::FromHDC(hdc);

		pGraphics->DrawImage(
			Image,
			destRect,
			imgRect,
			imgUnit,
			(const Gdiplus::ImageAttributes *)NULL
		);

#if 0
		std::wstringstream ss_sH;
		ss_sH << L"scaleH = " << std::setprecision(2) << scaleH;
		auto x_sH = ss_sH.str();
		TextOutW(hdc, 0, 0, x_sH.c_str(), (int)x_sH.length());
		std::wstringstream ss_sW;
		ss_sW << L"scaleW = " << std::setprecision(2) << scaleW;
		auto x_sW = ss_sW.str();
		TextOutW(hdc, 0, 16, x_sW.c_str(), (int)x_sW.length());
		std::wstringstream ss_clW;
		ss_clW << L"clW = " << clW;
		auto x_clW = ss_clW.str();
		TextOutW(hdc, 0, 32, x_clW.c_str(), (int)x_clW.length());
		std::wstringstream ss_clH;
		ss_clH << L"clH = " << clH;
		auto x_clH = ss_clH.str();
		TextOutW(hdc, 0, 48, x_clH.c_str(), (int)x_clH.length());
#endif
	
		delete pGraphics;
	}

	(void)EndPaint(hwnd, &ps);
}

LRESULT CALLBACK MainWindow::WndProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	if (uMsg == WM_NCCREATE)
	{
		SetLastError(0);
		if (!SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)(((LPCREATESTRUCTW)lParam)->lpCreateParams)))
			if (GetLastError())
				return FALSE;
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}

	// TODO: Assume this can't be NULL?
	auto this_ = (MainWindow *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

	switch (uMsg)
	{
		HANDLE_MSG(hwnd, WM_COMMAND,    this_->WmCommand);
		HANDLE_MSG(hwnd, WM_CREATE,     this_->WmCreate);
		HANDLE_MSG(hwnd, WM_CLOSE,      this_->WmClose);
		HANDLE_MSG(hwnd, WM_DROPFILES,  this_->WmDropFiles);
		HANDLE_MSG(hwnd, WM_SHOWWINDOW, this_->WmShowWindow);
		HANDLE_MSG(hwnd, WM_ENDSESSION, this_->WmEndSession);
		HANDLE_MSG(hwnd, WM_DESTROY,    this_->WmDestroy);
		HANDLE_MSG(hwnd, WM_PAINT,      this_->WmPaint);
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
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

	ATOM atom = RegisterClassExW(&wc);
	if (!atom)
		return false;

	if (!(accel_ = CreateAccelerators()))
		return false;

	hwnd_ = CreateWindowExW(
		WS_EX_ACCEPTFILES,
		(LPCWSTR)atom,
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
	{
		(void)DestroyAcceleratorTable(accel_);
		return false;
	}

	(void)ShowWindow(hwnd_, maximize ? SW_SHOWMAXIMIZED : SW_SHOW);
	(void)UpdateWindow(hwnd_);
	return true;
}

int MainWindow::Run()
{
	MSG msg;
	BOOL bRet;
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			// TODO: GetLastError() & log & cleanly shutdown any tasks (io, db, network, etc)
			// just exit for now though
			return -1;
		}

		if (!TranslateAcceleratorW(hwnd_, accel_, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	return (int)msg.wParam;
}

}
