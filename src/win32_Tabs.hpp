/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include <Windows.h>

namespace win32 {

class Tabbar
{
public:
	HWND hwnd_;
	HMENU context_menu_root_;
	HMENU context_menu_;
	int right_click_idx_;
	bool right_click_down_;

public:

	Tabbar() :
		hwnd_(NULL),
		right_click_idx_(-1),
		right_click_down_(false)
	{
		// hi
	}

	bool Init(HWND hParent);

	bool Append(const wchar_t * text);

	void Remove(int idx);

	HDWP DeferSize(HDWP hdwp, int cx, int cy);

	int TabUnderMouse();

	static LRESULT CALLBACK SubProc(
		HWND hwnd,
		UINT msg,
		WPARAM wParam,
		LPARAM lParam,
		UINT_PTR uIdSubclass,
		DWORD_PTR dwRefData
	);
};

}
