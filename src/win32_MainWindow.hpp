/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include <Windows.h>

namespace win32 {

class MainWindow
{
public:
	HWND hwnd_;
	HACCEL accel_;

public:

	void WmCommand(HWND hwnd, int id, HWND hwndCtrl, UINT codeNotify);

	BOOL WmCreate(HWND hwnd, LPCREATESTRUCT cs);

	void WmClose(HWND hwnd);

	void WmDropFiles(HWND hwnd, HDROP hdrop);

	void WmShowWindow(HWND hwnd, BOOL fShow, UINT status);

	void WmEndSession(HWND hwnd, BOOL fEnding);

	void WmDestroy(HWND hwnd);

	void WmPaint(HWND hwnd);

	static LRESULT CALLBACK WndProc(
		HWND hwnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam
	);

	HACCEL CreateAccelerators();

	bool Init(int w, int h, int x, int y, bool maximize);

	int Run();
};

}
