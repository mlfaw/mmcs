/*{REPLACEMEWITHLICENSE}*/
#include "win32_RegisterAsDefault.hpp"
#include "mmcs_globals.hpp"
#include <Windows.h>
#include <windowsx.h> // Button_GetCheck()
#include "generated/win32_resource.h"
#include "win32_hinstance.h"
#include "win32_misc.hpp" // win32::GetExePath()

namespace win32 {
namespace RegisterAsDefault {

static const wchar_t registerAsDefaultArg[] = L"mmcs://RegisterAsDefault";

struct registerData {
	bool jpg;
	bool png;
	bool gif;
	bool mp4;
	bool mkv;
	bool webm;
};

static struct registerData userSpecified;

static INT_PTR CALLBACK DialogProc(
	_In_ HWND   hwnd,
	_In_ UINT   msg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	switch (msg) {
	case WM_INITDIALOG:
	{
		HICON hIcon = LoadIconW(HINST_THISCOMPONENT, MAKEINTRESOURCEW(IDI_MMCS_ICON));
		(void)SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		(void)SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		return TRUE;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			userSpecified.jpg = Button_GetCheck(GetDlgItem(hwnd, IDC_JPG));
			userSpecified.png = Button_GetCheck(GetDlgItem(hwnd, IDC_PNG));
			userSpecified.gif = Button_GetCheck(GetDlgItem(hwnd, IDC_GIF));
			userSpecified.mp4 = Button_GetCheck(GetDlgItem(hwnd, IDC_MP4));
			userSpecified.mkv = Button_GetCheck(GetDlgItem(hwnd, IDC_MKV));
			userSpecified.webm = Button_GetCheck(GetDlgItem(hwnd, IDC_WEBM));
			(void)EndDialog(hwnd, IDOK);
			return TRUE;
		case IDCANCEL:
			(void)EndDialog(hwnd, IDCANCEL);
			return TRUE;
		}
	}
	return FALSE;
}

static void real_handler()
{
	INT_PTR ret = DialogBoxParamW(
		HINST_THISCOMPONENT,
		MAKEINTRESOURCEW(IDD_REGISTER_AS_DEFAULT),
		NULL, // hwndParent
		DialogProc,
		NULL // dwInitParam
	);

	if (ret != IDOK)
		return;

	// check content in userSpecified
}

bool Handler(const wchar_t * lpCmdLine)
{
	if (wcscmp(registerAsDefaultArg, lpCmdLine) != 0)
		return false;
	real_handler();
	return true;
}

void Launch()
{
	SHELLEXECUTEINFOW info = {0};
	info.cbSize = sizeof(info);
	info.lpVerb = L"runas";
	info.lpFile = win32::GetExePath();
	info.lpParameters = registerAsDefaultArg;
	info.nShow = SW_NORMAL;
	(void)ShellExecuteExW(&info);
}

}
}
