#include <Windows.h>

#include "TooltipWindow.hpp"

//#ifdef __cplusplus
//extern "C" {
//#endif

ATOM TooltipWindowClassAtom = 0;

LRESULT CALLBACK TooltipWindowProc(
    HWND   hwnd,
    UINT   msg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        {
            HWND button = CreateWindowExW(
                0, // dwExStyle
                L"BUTTON",
                L"closeplz",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                CW_USEDEFAULT, // x
                CW_USEDEFAULT, // y
                100, // width
                100, // height
                hwnd, // parent
                0, // hmenu
                (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE),
                NULL // lParam
            );

            HWND parent = GetParent(hwnd);
            ShowWindow(hwnd, SW_SHOW);
            EnableWindow(parent, FALSE);
            SetFocus(hwnd);
            break;
        }
    case WM_COMMAND:
        {
            HWND parent = GetParent(hwnd);
            EnableWindow(parent, TRUE);
            SetFocus(parent);
            DestroyWindow(hwnd);
            break;
        }
    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

unsigned InitTooltipWindowClass(HINSTANCE hInstance)
{
    WNDCLASSEXW tooltipWindowClass;

    if (TooltipWindowClassAtom != 0)
        return 1;

    tooltipWindowClass.cbSize = sizeof(tooltipWindowClass);

    tooltipWindowClass.style = CS_HREDRAW | CS_VREDRAW;
    tooltipWindowClass.lpfnWndProc = TooltipWindowProc;
    tooltipWindowClass.cbClsExtra = 0;
    tooltipWindowClass.cbWndExtra = 0;
    tooltipWindowClass.hInstance = hInstance;
    tooltipWindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    tooltipWindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    tooltipWindowClass.lpszMenuName = NULL;//MAKEINTRESOURCEW(IDC_WIN32PROJECT1);
    tooltipWindowClass.lpszClassName = L"tooltipWindowClass";
    tooltipWindowClass.hIcon = NULL;//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32PROJECT1));
    tooltipWindowClass.hIconSm = NULL;//LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    TooltipWindowClassAtom = RegisterClassExW(&tooltipWindowClass);

    return TooltipWindowClassAtom != 0;
}

//#ifdef __cplusplus
//}
//#endif