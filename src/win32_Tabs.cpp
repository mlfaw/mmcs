/*{REPLACEMEWITHLICENSE}*/
#include "win32_Tabs.hpp"
#include "generated/win32_resource.h"
#include "win32_hinstance.h"
#include <CommCtrl.h>
#include <windowsx.h>
#include "win32_gui.hpp"
#include "win32_TabPage.hpp"

namespace win32 {

bool Tabbar::Init(HWND hParent)
{
	HINSTANCE hInstance = HINST_THISCOMPONENT;
	RECT rc;

	if (!GetClientRect(hParent, &rc))
		return false;

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

	RECT rc2 = {};
	GetClientRect(hwnd_, &rc2);
	TabCtrl_AdjustRect(hwnd_, FALSE, &rc2);

	if (!win32::CreateTagPage(hParent, rc2.left, rc2.top, rc2.right - rc2.left, rc2.bottom - rc2.top))
		return false;

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
	case WM_MBUTTONDOWN:
		 tabbar->middle_button_down_ = true;
		 return 0;
	case WM_MBUTTONUP:
	{
		if (!tabbar->middle_button_down_)
			break;
		tabbar->middle_button_down_ = false;

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
	case WM_MOUSEWHEEL:
	case WM_MOUSEHWHEEL:
	{
		HWND updown = GetDlgItem(hwnd, 1); // the UpDown buttons... msctls_updown32
		if (!updown) break;
		short delta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (delta == 0) break;
		// invert the hscroll so it works how you'd expect
		if (msg == WM_MOUSEHWHEEL) delta = -delta;
		// save previous capture because updown will internally set it...
		HWND prev = SetCapture(updown);
		// delta > 0 = up
		// delta < 0 = down
		// updown control has up on the left (0 to w/2) and down on the right (w/2 or greater)
		// ... so lets use values that are definitely in those ranges...
		LPARAM x = delta > 0 ? 0 : 9999;
		SendMessageW(updown, WM_LBUTTONDOWN, 0, x);
		SetCapture(prev);
		return 0;
	}
	}

	return DefSubclassProc(hwnd, msg, wParam, lParam);
}

}

