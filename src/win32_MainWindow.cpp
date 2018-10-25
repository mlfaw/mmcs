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

static HACCEL MwCreateAccelerators(void)
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

static void MwWmCommand(HWND hwnd, int id, HWND hwndCtrl, UINT codeNotify)
{
	// Menu or Accelerator
	if (hwndCtrl == (HWND)0 || hwndCtrl == (HWND)1)
	{
		switch (id) {
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

static BOOL MwWmCreate(HWND hwnd, LPCREATESTRUCT cs)
{
	if (!win32::ImagePainter_Create(hwnd)) return FALSE;
	return TRUE;
}

static void MwWmClose(HWND hwnd)
{
	(void)DestroyWindow(hwnd);
}

static void MwWmEndSession(HWND hwnd, BOOL fEnding)
{
	PostQuitMessage(0);
}

static void MwWmDestroy(HWND hwnd)
{
	PostQuitMessage(0);
}

// NOTE: This handles all drop-files from child windows.
// A child window can handle drop-files that target them if they
// have the extended window style WS_EX_ACCEPTFILES
// or it has been enabled through DragAcceptFiles().
static void MwWmDropFiles(HWND hwnd, HDROP hdrop)
{
	POINT dropPoint;
	BOOL droppedInClientArea = DragQueryPoint(hdrop, &dropPoint);
	UINT fileCount = DragQueryFileW(hdrop, 0xFFFFFFFF, NULL, 0);

	DragFinish(hdrop);
}

static void MwWmShowWindow(HWND hwnd, BOOL fShow, UINT status)
{
}

static void MwWmSize(HWND hwnd, UINT state, int cx, int cy)
{
	if (state == SIZE_MINIMIZED) return;
	HWND hIp = GetDlgItem(hwnd, ID_IMAGEPAINTER);
	if (!hIp) return;
	(void)MoveWindow(hIp, 0, 0, cx, cy, TRUE);
}

static void MwWmSizing(HWND hwnd, UINT edge, RECT * rect)
{
	RECT rc;
	if (!GetClientRect(hwnd, &rc)) return;
	HWND fucker = GetDlgItem(hwnd, ID_IMAGEPAINTER);
	SendMessageW(fucker, WM_SIZING, (WPARAM)edge, (LPARAM)&rc);
}

static LRESULT CALLBACK MwWindowProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (uMsg)
	{
		HANDLE_MSG(hwnd, WM_CREATE, MwWmCreate);
		HANDLE_MSG(hwnd, WM_CLOSE, MwWmClose);
		HANDLE_MSG(hwnd, WM_DROPFILES, MwWmDropFiles);
		HANDLE_MSG(hwnd, WM_COMMAND, MwWmCommand);
		HANDLE_MSG(hwnd, WM_ENDSESSION, MwWmEndSession);
		HANDLE_MSG(hwnd, WM_DESTROY, MwWmDestroy);
		HANDLE_MSG(hwnd, WM_SHOWWINDOW, MwWmShowWindow);
		HANDLE_MSG(hwnd, WM_SIZE, MwWmSize);
	case WM_SIZING:
		MwWmSizing(hwnd, (UINT)wParam, (RECT *)lParam);
		return TRUE;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

namespace win32 {

int MwEverything(void)
{
	INITCOMMONCONTROLSEX controls;
	controls.dwSize = sizeof(controls);
	controls.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES | ICC_USEREX_CLASSES;
	if (!InitCommonControlsEx(&controls))
		return 1; // TODO: Log error

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

	HMENU mwMenu = LoadMenuW(hInstance, MAKEINTRESOURCEW(IDM_MAIN_MENU));

	HACCEL MwAccel = MwCreateAccelerators();

	WNDCLASSEXW wc;
	wc.cbSize = sizeof(wc);
	wc.style = 0;
	wc.lpfnWndProc = MwWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = iconMMCS;
	wc.hCursor = cursorArrow;
	wc.hbrBackground = (HBRUSH)(COLOR_GRAYTEXT + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"MainWindow";
	wc.hIconSm = NULL; // If NULL, it will try to get a smaller icon from hIcon

	ATOM MwAtom = RegisterClassExW(&wc);
	if (!MwAtom) return 1;

	HWND MwHwnd = CreateWindowExW(
		WS_EX_ACCEPTFILES,
		(LPCWSTR)MwAtom,
		L"MMCS by mlfaw",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, // x
		CW_USEDEFAULT, // y
		600,//CW_USEDEFAULT, // width
		400,//CW_USEDEFAULT, // height
		NULL, // hwndParent
		mwMenu,
		hInstance,
		NULL // lpParam
	);

	if (!MwHwnd) return 1;

	(void)ShowWindow(MwHwnd, SW_SHOW);
	(void)UpdateWindow(MwHwnd);

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

		if (!TranslateAcceleratorW(MwHwnd, MwAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	return (int)msg.wParam;
}

}
