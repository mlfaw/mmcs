#pragma once
#include <Windows.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

struct TooltipPage {
    const wchar_t * first;
    const wchar_t * second;
    const wchar_t * third;
    UINT ms;
};

LRESULT CALLBACK TooltipWindowProc(
    HWND   hwnd,
    UINT   msg,
    WPARAM wParam,
    LPARAM lParam);

unsigned InitTooltipWindowClass(HINSTANCE hInstance);

//void CreateTooltipWindow(
//    const wchar_t* caption,
//    );

extern ATOM TooltipWindowClassAtom;

//#ifdef __cplusplus
//}
//#endif
