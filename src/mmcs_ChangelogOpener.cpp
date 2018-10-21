/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_ChangelogOpener.hpp"
#include "generated/Changelog.h"
#include "mmcs_os.hpp"
#include "small_zip.h"
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#include <windowsx.h>
#include "generated/win32_resource.h"
#endif

namespace mmcs {

static const oschar * const file_name = _OS("mmcs_changelog_") MMCS_VERSION_UNDERSCORES ".txt";

#ifdef _WIN32
static INT_PTR CALLBACK DialogProc(
	_In_ HWND   hwnd,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	HICON hIcon;
	switch (uMsg) {
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
		Edit_SetText(GetDlgItem(hwnd, IDC_EDIT1), L"asjdfkljasdklfjadsfjklasjd;fjasd;fa\r\nsdfkljasdfj;asdjfklklklklklklklklklklklklklklklklklklkljas");
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			(void)EndDialog(hwnd, 0);
			return TRUE;
		}
		break;
	case WM_SIZE:
		{
			
			return TRUE;
		}
	}
	return FALSE;
}

bool WriteChangelog(osfile file_handle)
{
	unsigned char buf[2048];
	uint32_t bytes_decompressed;
	small_zip_ctx ctx;
	small_zip_file_ctx file_ctx;
	small_zip_decompress_ctx decompress_ctx;

	if (!small_zip_init(&ctx, GENERATED_Changelog_zip, GENERATED_Changelog_zip_size)) return false;
	if (!small_zip_file(&ctx, &file_ctx, 0)) return false; // grab the first file....
	if (!small_zip_decompress_init(&file_ctx, &decompress_ctx)) return false;
	while ((bytes_decompressed = small_zip_decompress(&decompress_ctx, buf, 2048))) {
		if (bytes_decompressed == -1) // error in compressed data
			return false;
#ifdef _WIN32
		DWORD bytes_written;
		if (!WriteFile(file_handle, buf, bytes_decompressed, &bytes_written, NULL))
			return false;
#else

#endif
	}

	return true;
}

void ChangelogHandler(void)
{
	// INT_PTR ret = DialogBoxParamW(
	// 	GetModuleHandle(NULL),
	// 	MAKEINTRESOURCEW(IDD_CHANGELOG),
	// 	hParent,
	// 	DialogProc,
	// 	NULL
	// );
	// (void)ret;

#ifdef _WIN32
	wchar_t temppath[MAX_PATH+1];
	DWORD nChars = GetTempPathW(MAX_PATH+1, temppath);
	if (!nChars || nChars > MAX_PATH+1) return; // TODO: Log?
	auto x = osstrlen(file_name);
	if (x + nChars > MAX_PATH) return;
	osstrcat(temppath, file_name);
	//if (PathCchAppend(temppath, MAX_PATH+1, file_name) != S_OK) return;

	HANDLE hFile = CreateFileW(
		temppath,
		GENERIC_WRITE,
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS, //CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	// Time to extract...
	if (hFile != INVALID_HANDLE_VALUE) {
		// Unzip and write...
		bool success = WriteChangelog(hFile);
		CloseHandle(hFile);
		// Well, let's not leave an invalid changelog on disk...
		if (!success) {
			(void)DeleteFileW(temppath);
			return;
		}
	}

	int ret = (int)(INT_PTR)ShellExecuteW(NULL, L"open", temppath, NULL, NULL, SW_SHOWNORMAL);
	if (ret > 32) {
		// success
	}
#endif
}
#endif

}
