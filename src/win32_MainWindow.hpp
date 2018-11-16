/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include <Windows.h>
#include "win32_Tabs.hpp"

namespace win32 {

class MainWindow
{
public:
	HWND hwnd_;
	win32::Tabbar tabbar_;

public:

	MainWindow() :
		hwnd_(NULL)
	{
		// hi
	}
	
	// TODO: Destructor that deallocates data?


	BOOL WmCreate(HWND hwnd, LPCREATESTRUCT cs);

	void WmClose(HWND hwnd);

	void WmDestroy(HWND hwnd);

	void WmEndSession(HWND hwnd, BOOL fEnding);

	void WmCommand(HWND hwnd, int id, HWND hwndCtrl, UINT codeNotify);

	LRESULT WmNotify(HWND hwnd, int ctrlId, NMHDR * info);

	void WmDropFiles(HWND hwnd, HDROP hdrop);

	void WmWindowPosChanged(HWND hwnd, const LPWINDOWPOS lpwpos);

	void WmSize(HWND hwnd, UINT state, int cx, int cy);

	static LRESULT CALLBACK WndProc(
		HWND hwnd,
		UINT msg,
		WPARAM wParam,
		LPARAM lParam
	);

	int Run();

	bool Init(int w, int h, int x, int y, bool maximize);
};

}
