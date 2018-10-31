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

HDWP Tabbar::DeferSize(HDWP hdwp, int cx, int cy)
{
	hdwp = DeferWindowPos(
		hdwp,
		hwnd_,
		NULL, // hWndInsertAfter
		0, // x
		0, // y
		cx,
		cy,
		SWP_NOZORDER
	);

	if (!hdwp)
		return NULL;


	return hdwp;
}

int Tabbar::TabUnderMouse(POINT * out)
{
	POINT point;

	if (!GetCursorPos(&point))
		return -1;

	if (!ScreenToClient(hwnd_, &point))
		return -1;

	TCHITTESTINFO hittest;
	hittest.pt = point;
	int idx = TabCtrl_HitTest(hwnd_, &hittest);
	if (idx != -1 && out) *out = point;
	return idx;
}

LRESULT CALLBACK Tabbar::SubProc(
	HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam,
	UINT_PTR uIdSubclass,
	DWORD_PTR dwRefData
)
{
	Tabbar * tabbar = (Tabbar *)dwRefData;

	switch (msg)
	{
	case WM_NCDESTROY:
	{
		HMENU x = tabbar->context_menu_root_;
		if (x)
			(void)DestroyMenu(x);
		(void)RemoveWindowSubclass(hwnd, SubProc, 0);
		break;
	}
	case WM_MBUTTONUP:
	{
		int idx = tabbar->TabUnderMouse(NULL);
		if (idx == -1)
			break;

		tabbar->Remove(idx);
		return 0;
	}
	case WM_RBUTTONDOWN:
		tabbar->right_click_down_ = true;
		return 0;
	case WM_RBUTTONUP:
	{
		// I want to keep tab handling here instead of using NM_RCLICK in MainWindow...
		if (!tabbar->right_click_down_)
			break;
		tabbar->right_click_down_ = false;

		POINT point;
		int idx = tabbar->TabUnderMouse(&point);
		if (idx == -1)
			break;
	
		tabbar->right_click_idx_ = idx;

		auto x = DefWindowProcW;

		BOOL ret = TrackPopupMenu(
			tabbar->context_menu_,
			0, // flags
			point.x,
			point.y,
			0, // reserved
			GetParent(hwnd),
			NULL // prcRect
		);

		if (!ret) tabbar->right_click_idx_ = -1;
		return 0;
	}
	}

	// tab scrolling is currently broken
#if 0
	if (msg != WM_MOUSEWHEEL && msg != WM_MOUSEHWHEEL)
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

	return DefSubclassProc(hwnd, msg, wParam, lParam);
}

}

