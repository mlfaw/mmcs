/*{REPLACEMEWITHLICENSE}*/
#pragma once
#include <Windows.h>
#include <gdiplus.h>
#include "win32_Tabs.hpp"

namespace win32 {

class MainWindow
{
public:
	HWND hwnd_;
	HACCEL accel_;

	win32::Tabbar tabbar_;

	Gdiplus::Image * image_;
	bool fitted_;
	double image_scale_;

public:

	MainWindow() :
		hwnd_(NULL),
		accel_(NULL),
		fitted_(true),
		image_scale_(1.0)
	{
		// hi
	}
	
	// TODO: Destructor that deallocates data?

	void WmCommand(HWND hwnd, int id, HWND hwndCtrl, UINT codeNotify);

	LRESULT WmNotify(HWND hwnd, int ctrlId, NMHDR * info);

	BOOL WmCreate(HWND hwnd, LPCREATESTRUCT cs);

	void WmClose(HWND hwnd);

	void WmDropFiles(HWND hwnd, HDROP hdrop);

	void WmShowWindow(HWND hwnd, BOOL fShow, UINT status);

	void WmEndSession(HWND hwnd, BOOL fEnding);

	void WmDestroy(HWND hwnd);

	void GetImageLocation(
		Gdiplus::RectF & destRect,
		Gdiplus::RectF & imgRect,
		double imgW,
		double imgH,
		double clW,
		double clH
	);

	void WmPaint(HWND hwnd);

	void WmSize(HWND hwnd, UINT state, int cx, int cy);

	void WmMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys);

	static LRESULT CALLBACK WndProc(
		HWND hwnd,
		UINT msg,
		WPARAM wParam,
		LPARAM lParam
	);

	static bool DoDispatchCheck(MSG * msg, void * user_data);

	int Run();

	HACCEL CreateAccelerators();

	bool Init(int w, int h, int x, int y, bool maximize);
};

}
