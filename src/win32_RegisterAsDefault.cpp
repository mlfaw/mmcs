/*{REPLACEMEWITHLICENSE}*/
#include "win32_RegisterAsDefault.hpp"
#include "mmcs_globals.hpp"
#include <Windows.h>
#include <windowsx.h> // Button_GetCheck()
#include "generated/win32_resource.h"

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
		hIcon = (HICON)LoadImageW(
			GetModuleHandle(NULL),
			MAKEINTRESOURCEW(IDI_MMCS_ICON),
			IMAGE_ICON,
			0,
			0,
			LR_SHARED
		);
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
		GetModuleHandle(NULL),
		MAKEINTRESOURCEW(IDD_REGISTER_AS_DEFAULT),
		NULL,
		DialogProc,
		NULL
	);

	if (ret == 2) return 0; // cancel button
	if (ret != 1) return 1; // don't know...
	// ok button

	// check content inside userSpecified

	return 0;
}

void RegisterAsDefault_Launch()
{
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

	SHELLEXECUTEINFOW info = {0};
	info.cbSize = sizeof(info);
	info.lpVerb = L"runas";
	info.lpFile = mmcs::ExePath;
	info.lpParameters = REGISTER_AS_DEFAULT_ARG;
	info.nShow = SW_NORMAL;
	(void)ShellExecuteExW(&info);
}

}
