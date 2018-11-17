/*{REPLACEMEWITHLICENSE}*/
#include "win32_MainWindow.hpp"
#include <commctrl.h> // WM_NOTIFY, TCN_SELCHANGING, TCN_SELCHANGE
#include <shellapi.h> // DragAcceptFiles(), DragFinish(), DragQueryPoint(), DragQueryFileW()
// "Message crackers" and other helpful functions
// https://docs.microsoft.com/en-us/windows/desktop/api/windowsx/
#include <windowsx.h>

#include "generated/win32_resource.h"
#include "win32_RegisterAsDefault.hpp"
#include "mmcs_Open.hpp" // OpenUrlAsync(), OpenFileAsync()
#include "mmcs_os.hpp"
#include "mmcs_GetDirectoryFiles.hpp" // std::vector, std::wstring..
#include "mmcs_globals.hpp"
#include "win32_gui.hpp"
#include "win32_hinstance.h"
#include "mmcs_SelectFilesWindow.hpp"
#include "win32_ImagePainter.hpp"

namespace win32 {

BOOL MainWindow::WmCreate(HWND hwnd, LPCREATESTRUCT cs)
{
	if (!win32::ImagePainter_Create(hwnd))
		return FALSE;
#if 0
	if (!tabbar_.Init(hwnd))
		return FALSE;
#endif
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

void MainWindow::WmDestroy(HWND hwnd)
{
	PostQuitMessage(0);
}

void MainWindow::WmEndSession(HWND hwnd, BOOL fEnding)
{
	// TODO:
}

void MainWindow::WmCommand(HWND hwnd, int id, HWND hwndCtrl, UINT codeNotify)
{
	if (hwndCtrl != (HWND)0 && hwndCtrl != (HWND)1)
	{
		// actual control window
		return;
	}

	// Menu or Accelerator
	switch (id)
	{
	case IDM_FILE_NEW_TAB:
		tabbar_.Append(L"files (0)");
		break;
	case IDM_CLOSE_TAB:
	{
		int idx = tabbar_.right_click_idx_;
		if (idx == -1)
			// Coming from Ctrl+W if invalid right_click_idx_
			if (-1 == (idx = tabbar_.GetSel()))
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
		if (mmcs::isPortable) {
			(void)MessageBoxW(
				NULL,
				(L"MMCS is running in portable mode.\n"
				L"Registering as the default program for media is not available."),
				L"Register As Default",
				MB_OK
			);
			return;
		}
		win32::RegisterAsDefault::Launch();
		break;
	case IDM_HELP_ABOUT:
		break;
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

#define SWP_STATECHANGED 0x8000
#define SWP_NOCLIENTSIZE 0x0800
#define SWP_NOCLIENTMOVE 0x1000
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

#if 0
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
#endif

#if 0
	HWND imagePainter = GetDlgItem(hwnd, ID_IMAGEPAINTER);
	if (imagePainter)
	{
		hdwp = DeferWindowPos(
			hdwp,
			imagePainter,
			NULL,
			0,
			0,
			0,
			0,
			SWP_NOZORDER | SWP_NOSIZE
		);
		if (!hdwp) return;
	}
#endif

	(void)EndDeferWindowPos(hdwp);

#if 0
	if (imagePainter)
		RedrawWindow(imagePainter, NULL, NULL, RDW_UPDATENOW | RDW_INVALIDATE);
#endif
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

	if (mw)
	{
		switch (msg)
		{
			HANDLE_MSG(hwnd, WM_CREATE,     mw->WmCreate);
			HANDLE_MSG(hwnd, WM_CLOSE,      mw->WmClose);
			HANDLE_MSG(hwnd, WM_DESTROY,    mw->WmDestroy);
			HANDLE_MSG(hwnd, WM_ENDSESSION, mw->WmEndSession);
			HANDLE_MSG(hwnd, WM_COMMAND,    mw->WmCommand);
			HANDLE_MSG(hwnd, WM_NOTIFY,     mw->WmNotify);
			HANDLE_MSG(hwnd, WM_DROPFILES,  mw->WmDropFiles);
			HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGED, mw->WmWindowPosChanged);
#if 0
		case WM_SIZING:
		{
			RECT * prc = (RECT *)lParam;
			mw->WmSize(hwnd, 0, prc->right, prc->bottom);
			return TRUE;
		}
#endif
		}
	}

	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int MainWindow::Run()
{
	MSG msg;
	BOOL bRet;
	while ((bRet = GetMessageW(&msg, NULL, 0, 0)) != 0)
	{
		if (bRet == -1)
			return -1;

		if (!TranslateAcceleratorW(hwnd_, MainWindowAccelerators, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
	return (int)msg.wParam;
}

bool MainWindow::Init(int w, int h, int x, int y, bool maximize)
{
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
		HINST_THISCOMPONENT,
		this // lpParam
	);

	if (!hwnd_)
		return false;

	if (mmcs::isPortable)
	{
		MENUITEMINFOW item = {};
		item.cbSize = sizeof(item);
		item.fMask = MIIM_STATE;
		item.fState = MFS_DISABLED;
		(void)SetMenuItemInfoW(GetMenu(hwnd_), IDM_HELP_REGISTER_AS_DEFAULT, FALSE, &item);
	}

	(void)ShowWindow(hwnd_, maximize ? SW_SHOWMAXIMIZED : SW_SHOW);
	(void)UpdateWindow(hwnd_);
	return true;
}

}
