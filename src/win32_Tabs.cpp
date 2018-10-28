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

}

