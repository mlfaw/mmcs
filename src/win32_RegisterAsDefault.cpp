/*{REPLACEMEWITHLICENSE}*/
#include "win32_RegisterAsDefault.hpp"
#include "mmcs_globals.hpp"
#include <Windows.h>
#include <windowsx.h> // Button_GetCheck()
#include "generated/win32_resource.h"
#include "win32_hinstance.h"

namespace win32 {

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
	HICON hIcon;
	struct registerData * d;
	switch (msg) {
	case WM_INITDIALOG:
		hIcon = LoadIconW(HINST_THISCOMPONENT, MAKEINTRESOURCEW(IDI_MMCS_ICON));
		(void)SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		(void)SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			d = &userSpecified;
			d->jpg = Button_GetCheck(GetDlgItem(hwnd, IDC_JPG));
			d->png = Button_GetCheck(GetDlgItem(hwnd, IDC_PNG));
			d->gif = Button_GetCheck(GetDlgItem(hwnd, IDC_GIF));
			d->mp4 = Button_GetCheck(GetDlgItem(hwnd, IDC_MP4));
			d->mkv = Button_GetCheck(GetDlgItem(hwnd, IDC_MKV));
			d->webm = Button_GetCheck(GetDlgItem(hwnd, IDC_WEBM));
			(void)EndDialog(hwnd, 1);
			return TRUE;
		case IDCANCEL:
			(void)EndDialog(hwnd, 2);
			return TRUE;
		}
	}
	return FALSE;
}

int RegisterAsDefault_Handler()
{
	INT_PTR ret = DialogBoxParamW(
		HINST_THISCOMPONENT,
		MAKEINTRESOURCEW(IDD_REGISTER_AS_DEFAULT),
		NULL, // hwndParent
		DialogProc,
		NULL // dwInitParam
	);

	if (ret == 2) return 0; // cancel button
	if (ret != 1) return 1; // don't know...
	// ok button

	// check content inside userSpecified

	return 0;
}

void RegisterAsDefault_Launch()
{
	SHELLEXECUTEINFOW info = {0};
	info.cbSize = sizeof(info);
	info.lpVerb = L"runas";
	info.lpFile = mmcs::ExePath;
	info.lpParameters = REGISTER_AS_DEFAULT_ARG;
	info.nShow = SW_NORMAL;
	(void)ShellExecuteExW(&info);
}

}
