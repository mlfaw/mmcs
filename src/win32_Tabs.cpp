/*{REPLACEMEWITHLICENSE}*/
#include "win32_Tabs.hpp"
#include "generated/win32_resource.h"
#include "win32_hinstance.h"
#include <CommCtrl.h>
#include <windowsx.h>
#include "win32_gui.hpp"

namespace win32 {

bool Tabbar::Init(HWND hParent)
{
	HINSTANCE hInstance = HINST_THISCOMPONENT;
	RECT rc;

	GetClientRect(hParent, &rc);

	hwnd_ = CreateWindowExW(
		0,
		WC_TABCONTROL,
		L"",
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | TCS_HOTTRACK | TCS_FLATBUTTONS,
		0, // x
		0, // y
		rc.right, // w
		rc.bottom, // h
		hParent,
		(HMENU)ID_TABBAR,
		hInstance,
		NULL // lpParam
	);

	if (!hwnd_)
		return false;

	context_menu_root_ = LoadMenuW(hInstance, MAKEINTRESOURCEW(IDM_TAB_MENU));
	if (!context_menu_root_)
		goto err;
	context_menu_ = GetSubMenu(context_menu_root_, 0);

	if (!SetWindowSubclass(hwnd_, SubProc, 0, (DWORD_PTR)this))
		goto err;

	if (!Append(L"files(1)"))
		goto err;

	return true;

err:
	if (context_menu_root_)
		DestroyMenu(context_menu_root_);
	if (hwnd_)
		DestroyWindow(hwnd_);
	return false;
}

bool Tabbar::Append(const wchar_t * text)
{	
	TCITEMW tie = {};
	tie.mask = TCIF_TEXT;
	tie.pszText = (LPWSTR)text;
	return -1 != TabCtrl_InsertItem(hwnd_, TabCtrl_GetItemCount(hwnd_), &tie);
}

void Tabbar::Remove(int idx)
{
	int sel = TabCtrl_GetCurSel(hwnd_);
	TabCtrl_DeleteItem(hwnd_, idx);
	int count = TabCtrl_GetItemCount(hwnd_);
	if (sel == idx && count)
	{
		// keep the same index because i prefer the same index over the previous tab...

		if (count == sel) --sel; // if we were the last-tab, make sure we select the new last-tab
		TabCtrl_SetCurSel(hwnd_, sel);
	}
}

LRESULT CALLBACK Tabbar::SubProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	UINT_PTR uIdSubclass,
	DWORD_PTR dwRefData
)
{
	if (uMsg == WM_NCDESTROY)
	{
		RemoveWindowSubclass(hwnd, SubProc, 0);
		goto def;
	}

	if (uMsg == WM_MBUTTONUP)
	{
		POINT point;

		if (!GetCursorPos(&point))
			goto def;

		if (!ScreenToClient(hwnd, &point))
			goto def;

		TCHITTESTINFO hittest;
		hittest.pt = point;
		int idx = TabCtrl_HitTest(hwnd, &hittest);

		if (idx == -1)
			goto def;

		((Tabbar *)dwRefData)->Remove(idx);
		return 0;
	}

	// tab scrolling is currently broken
#if 0
	if (uMsg != WM_MOUSEWHEEL && uMsg != WM_MOUSEHWHEEL)
		goto def;

	HWND buttons = GetDlgItem(hwnd, 1); // the UpDown buttons... msctls_updown32
	if (!buttons)
		goto def;
	short delta = GET_WHEEL_DELTA_WPARAM(wParam);
	if (delta == 0)
		goto def;

	if (delta)
	{
		WPARAM x = (WPARAM)((1 << 16) | SB_THUMBPOSITION);
		SendMessageW(hwnd, WM_HSCROLL, x, (LPARAM)buttons);
		SendMessageW(hwnd, WM_HSCROLL, (WPARAM)SB_ENDSCROLL, (LPARAM)buttons);
	}
#endif

def:
	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

}

