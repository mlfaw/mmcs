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

public:

	Tabbar() :
		hwnd_(NULL),
		right_click_idx_(-1)
	{
		// hi
	}

	bool Init(HWND hParent);

	bool Append(const wchar_t * text);

	void Remove(int idx);
};

}
