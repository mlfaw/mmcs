/*{REPLACEMEWITHLICENSE}*/
#include "win32_TabPage.hpp"
#include "win32_hinstance.h"
#include "generated/win32_resource.h"

namespace win32 {

LRESULT CALLBACK TabPageWndProc(
	HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (msg)
	{
	case WM_PAINT:
	{

	}
	}
	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

HWND CreateTagPage(HWND hParent, int x, int y, int w, int h)
{
	HINSTANCE hInstance = HINST_THISCOMPONENT;//mmcs::hInstance;//(HINSTANCE)GetModuleHandleW(NULL);

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
	wc.lpfnWndProc = TabPageWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = cursorArrow;
	wc.hbrBackground = (HBRUSH)(COLOR_GRAYTEXT + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"TabPage";
	wc.hIconSm = NULL; // If NULL, it will try to get a smaller icon from hIcon
	if (!RegisterClassExW(&wc))
		return NULL;

	HWND hwnd = CreateWindowExW(
		0, // WS_EX_WINDOWEDGE,
		L"TabPage",
		L"",
		WS_CHILD | WS_VISIBLE, // WS_CLIPSIBLINGS?
		x,
		y,
		w,
		h,
		hParent,
		(HMENU)ID_TAB_PAGE,
		hInstance,
		NULL
	);

	return hwnd;
}

}
