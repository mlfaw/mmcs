// Random notes:
// "Mw" = Main Window.
// Icon sizes: 16, 20, 24, 32, 40, 48, 64, 96, 128, 256, 512(why not?) And more go back to those bookmarks

#include <Windows.h>

#include <Commctrl.h>

#ifdef __cplusplus
//#include <iostream>
#include <vector>
#include <string>
#endif
#include <string.h>
#include <stdlib.h>
//#include <stdio.h>
#include <stdint.h>

#include "wWinMain.hpp"

#ifdef __cplusplus
#include "nlohmann/json/src/json.hpp"
#endif

//#include "../out.c"

//#pragma section(".text", shared, read, execute)
//#pragma section(".rdata", shared, read)
#ifdef __cplusplus
struct VersionState {
    const wchar_t* version;
    const wchar_t* date;
    // Pointers to multiple strings.
    // After every NUL there's another string.
    // Terminates at double-NUL.
    const wchar_t* urgent;
    const wchar_t* fix;
    const wchar_t* new_ugh; // fuck you c++
    const wchar_t* misc;
};

struct ReasonableVersionState {
    std::wstring version;
    std::wstring date;

    std::vector<std::wstring> urgent;
    std::vector<std::wstring> fix;
    std::vector<std::wstring> new_ugh; // fuck you c++
    std::vector<std::wstring> misc;
};
#endif


#ifdef __cplusplus
namespace mymcs {
struct VersionState {
    std::wstring version;
    std::wstring date;

    std::vector<std::wstring> urgent;
    std::vector<std::wstring> fix;
    std::vector<std::wstring> new_ugh; // fuck you c++
    std::vector<std::wstring> misc;
};

#if 0
void to_json(json& j, const person& p) {
    j = json{ { "name", p.name },{ "address", p.address },{ "age", p.age } };
}
#endif

void from_json(const nlohmann::json& j, VersionState& vstate) {
    vstate.version = j.at("version").get<std::wstring>();
    vstate.date = j.at("date").get<std::wstring>();
    vstate.urgent = j.at("urgent").get<std::vector<std::wstring>>();
    vstate.fix = j.at("fix").get<std::vector<std::wstring>>();
    vstate.new_ugh = j.at("new").get<std::vector<std::wstring>>();
    vstate.misc = j.at("misc").get<std::vector<std::wstring>>();
}
}
#endif

//extern "C" {
//#include "TooltipWindow.hpp"
//}

/////////////////////////
// Global Variables
HINSTANCE g_hInst = NULL;

wchar_t* WinMainCmdline = NULL;
wchar_t* FullCmdline = NULL;
int My_argc = 0;
wchar_t** My_argv = NULL; // LocalFree() me if needed

HWND MwHwnd = NULL;
ATOM MwClassAtom = 0;

// Updates windows stuff
HWND UpdatesHwnd = NULL; // Contains the update-list container, and edit thing.
HWND UpdatesContainerList = NULL;
HWND* UpdatesContainerItems = NULL;
HWND UpdatesEditBox = NULL;
ATOM UpdatesClassAtom = 0;
ATOM UpdatesContainerListClassAtom = 0;
ATOM UpdatesContainerItemClassAtom = 0;

//const int Updates

const int UpdatesWindow_width = 500;
const int UpdatesWindow_height = 500;
const int UpdatesWindow_bottom_buffer = 100;

const int UpdatesContainerList_x = 0;
const int UpdatesContainerList_y = 100;
const int UpdatesContainerList_width = 200;
const int UpdatesContainerList_height = 0;//UpdatesWindow_height - UpdatesContainerList_y - UpdatesWindow_bottom_buffer;

const int UpdatesContainerItem_width = 200;
const int UpdatesContainerItem_height = 50;

const int UpdatesEditBox_x = 0;//UpdatesWindow_width - UpdatesContainerList_width - 90;
const int UpdatesEditBox_y = 100;
const int UpdatesEditBox_width = 0;//UpdatesWindow_width - UpdatesContainerList_width - 20;
const int UpdatesEditBox_height = 0;//UpdatesWindow_height - UpdatesEditBox_y - UpdatesWindow_bottom_buffer;


// About window that is hidden until we want it.
HWND AboutHwnd = NULL;
ATOM AboutClassAtom = 0;



HWND TestButtonHwnd = NULL;

// DrawMenuBar(mainWindow or whatever) must be used whenever a menu bar/item is updated.
HMENU MwMenuBar = NULL;
HMENU MwMenuFile = NULL;
HMENU MwMenuHelp = NULL;
HMENU MwMenuFilePopupTest = NULL;

/////////////////////////
// Window (buttons) IDs
#define TestButtonId 110

enum MyIds {
    IDC_NOTHING = 100,
    
    // Stuff in the Mw File menubar item.
    IDM_MW_FILE_NEW,
    IDM_MW_FILE_A,
    IDM_MW_FILE_B,
    IDM_MW_FILE_C,
    IDM_MW_FILE_D,
    IDM_MW_FILE_E,
    IDM_MW_FILE_F,
    IDM_MW_FILE_G,
    IDM_MW_FILE_H,
    IDM_MW_FILE_I,
    IDM_MW_FILE_J,
    IDM_MW_FILE_K,
    IDM_MW_FILE_L,
    IDM_MW_FILE_M,
    IDM_MW_FILE_POPUPTEST_HEY,

    // Stuff in the Mw Edit menubar item.
    IDM_MW_EDIT_A,
    IDM_MW_EDIT_B,
    IDM_MW_EDIT_C,
    IDM_MW_EDIT_D,
    IDM_MW_EDIT_E,
    IDM_MW_EDIT_F,
    IDM_MW_EDIT_G,
    IDM_MW_EDIT_H,
    IDM_MW_EDIT_I,
    IDM_MW_EDIT_J,
    IDM_MW_EDIT_K,
    IDM_MW_EDIT_L,
    IDM_MW_EDIT_M,

    // Stuff in the Mw Help menubar item.
    IDM_MW_HELP_ABOUT,
    IDM_MW_HELP_UPDATES,

    ID_UPDATES_
};

#ifdef __cplusplus
const char* json_single= R"(
    {
        "version": "255",
        "date": "2017-07-27",
        "urgent": [
            "Patched a code execution exploit"
        ],
        "fix": [
            "Fixed the taskbar icon from blahing",
            "Fixed a broken downloader"
        ],
        "new": [
            "Added support for Pixiv"
        ],
        "misc": [
        ]
    }
)";

const char* json_multiple = R"([
    {
        "version": "255",
        "date": "2017-07-27",
        "urgent": [
            "Patched a code execution exploit"
        ],
        "fix": [
            "Fixed the taskbar icon from blahing",
            "Fixed a broken downloader"
        ],
        "new": [
            "Added support for Pixiv"
        ],
        "misc": [
        ]
    },
    {
        "version": "254",
        "date": "2017-07-20",
        "urgent": [
            "This is a test"
        ],
        "fix": [
        ],
        "new": [
            "Added support for some random booru"
        ],
        "misc": [
            "Moved a button control"
        ]
    }
])";
#endif

/////////////////////////
// Functions

LRESULT CALLBACK BlankWinProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

#if 0
void ConvertStrToWstr(const char* s, std::wstring& out)
{
    int r = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, NULL, 0);
    if (r == 0)
        throw;
    out.reserve(r);
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, &out[0], r);
}

std::vector<struct ReasonableVersionState>
GenerateReasonableVersionStates(nlohmann::json j)
{
    std::vector<struct ReasonableVersionState> vec;
    for (const auto& elem : j) {
        struct ReasonableVersionState vstate;
        ConvertStrToWstr(elem.at("version").get<std::string>().c_str(), vstate.version);
        ConvertStrToWstr(elem.at("date").get<std::string>().c_str(), vstate.date);
        for (const auto& k : elem.at("urgent").get<std::vector<std::string>>()) {
            std::wstring asdf;
            ConvertStrToWstr(k.c_str(), asdf);
            vstate.urgent.push_back(asdf);
        }
        for (const auto& k : elem.at("fix").get<std::vector<std::string>>()) {
            std::wstring asdf;
            ConvertStrToWstr(k.c_str(), asdf);
            vstate.fix.push_back(asdf);
        }
        for (const auto& k : elem.at("new").get<std::vector<std::string>>()) {
            std::wstring asdf;
            ConvertStrToWstr(k.c_str(), asdf);
            vstate.new_ugh.push_back(asdf);
        }
        for (const auto& k : elem.at("misc").get<std::vector<std::string>>()) {
            std::wstring asdf;
            ConvertStrToWstr(k.c_str(), asdf);
            vstate.misc.push_back(asdf);
        }
        vec.push_back(vstate);
    }
    return vec;
}

struct VersionState*
GenerateVersionState(const char* s)
{
    struct VersionState* vstate = NULL;
    unsigned alloc_size = 0;

    try {
        auto j = nlohmann::json::parse(s);
        auto version = j.at("version").get<std::string>();
        auto date = j.at("date").get<std::string>();
        auto urgent = j.at("urgent").get<std::vector<std::string>>();
        auto fix = j.at("fix").get<std::vector<std::string>>();
        auto new_ugh = j.at("new").get<std::vector<std::string>>();
        auto misc = j.at("misc").get<std::vector<std::string>>();

#define UtfMacroLenThing(x) \
    do { \
        alloc_size += \
            2 * (r = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (x).c_str(), -1, NULL, 0)); \
        if (r == 0) \
            return NULL; \
    } while (0)

        int r = 0;
        UtfMacroLenThing(version);
        UtfMacroLenThing(date);
        for (const auto& elem : urgent) {
            UtfMacroLenThing(elem);
        }
        for (const auto& elem : fix) {
            UtfMacroLenThing(elem);
        }
        for (const auto& elem : new_ugh) {
            UtfMacroLenThing(elem);
        }
        for (const auto& elem : misc) {
            UtfMacroLenThing(elem);
        }
#undef UtfMacroLenThing

        // the 8 is for the extra NUL terminators
        vstate = (struct VersionState*)HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            alloc_size + sizeof(struct VersionState) + 8
        );
        if (!vstate)
            return NULL;

        wchar_t* buf = (wchar_t*)(vstate + 1);

        alloc_size /= 2;
        r = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, version.c_str(), -1, buf, alloc_size);
        vstate->version = buf;
        alloc_size -= r;
        buf += r;
        r = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, date.c_str(), -1, buf, alloc_size);
        vstate->date = buf;
        alloc_size -= r;
        buf += r;
        if (urgent.size())
            vstate->urgent = buf;
        for (const auto& elem : urgent) {
            r = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, elem.c_str(), -1, buf, alloc_size + 1);
            alloc_size -= r;
            buf += r;
        }
        *buf++ = 0;
        if (fix.size())
            vstate->fix = buf;
        for (const auto& elem : fix) {
            r = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, elem.c_str(), -1, buf, alloc_size + 1);
            alloc_size -= r;
            buf += r;
        }
        *buf++ = 0;
        if (new_ugh.size())
            vstate->new_ugh = buf;
        for (const auto& elem : new_ugh) {
            r = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, elem.c_str(), -1, buf, alloc_size + 1);
            alloc_size -= r;
            buf += r;
        }
        *buf++ = 0;
        if (misc.size())
            vstate->misc = buf;
        for (const auto& elem : misc) {
            r = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, elem.c_str(), -1, buf, alloc_size + 1);
            alloc_size -= r;
            buf += r;
        }
        *buf++ = 0;
    } catch (...) {
        return NULL;
    }
    
    return vstate;
}

void FreeVersionState(struct VersionState* vstate)
{
    HeapFree(GetProcessHeap(), 0, vstate);
}
#endif

// limit the minimum and maximum size.
LRESULT MinMaxXandYonWindow(LPARAM lParam, LONG min_x, LONG min_y, LONG max_x, LONG max_y)
{
    MINMAXINFO* mmi = (MINMAXINFO*)lParam;
    mmi->ptMinTrackSize.x = min_x;
    mmi->ptMinTrackSize.y = min_y;
    mmi->ptMaxTrackSize.x = max_x;
    mmi->ptMaxTrackSize.y = max_y;
    return 0;
}

// disable window borders from being user-resizable or changing the cursor
LRESULT DisableResizableWindowBorders(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    LRESULT ret = DefWindowProcW(hwnd, msg, wParam, lParam);
    switch (ret)
    {
    case HTBOTTOM:
    case HTBOTTOMLEFT:
    case HTBOTTOMRIGHT:
    case HTLEFT:
    case HTRIGHT:
    case HTTOP:
    case HTTOPLEFT:
    case HTTOPRIGHT:
        return HTCLIENT;
    }
    return ret;
}

void StartTestTooltip(HWND hwnd)
{
#if 0
    struct TooltipPage pages[3];


    pages[0].ms = 5000;
    pages[0].first = L"";
    pages[0].second = L"";
    pages[0].third = NULL;

    pages[1].ms = 5000;
    pages[1].first = NULL;
    pages[1].second = L"";
    pages[1].third = NULL;

    pages[2].ms = 5000;
    pages[2].first = L"";
    pages[2].second = L"";
    pages[2].third = L"";

    HWND popup = CreateWindowExW(
        0, // dwExStyle
        (LPCWSTR)TooltipWindowClassAtom,
        L"Window name here",
        WS_POPUP | WS_CAPTION | WS_VISIBLE,
        CW_USEDEFAULT, // x
        CW_USEDEFAULT, // y
        200, // width
        200, // height
        hwnd, // parent
        0, // hmenu
        g_hInst,
        NULL
    );

    int error = (int)GetLastError();
#endif
}

// Was used to check how much memory is shared (Copy-on-write) between processes.
// Seems to be everything that can be shared like const data and exec pages.
// Each process does get something like 1MB of thread stack space though.
DWORD WINAPI ProcessesTest(LPVOID lpParameter)
{
#if 0
    return 0;
#else
    wchar_t modulebuf[255] = {0};
    GetModuleFileName(NULL, modulebuf, 255);
    PROCESS_INFORMATION pi;
    STARTUPINFOW info = { 0 };
    info.cb = sizeof(info);
    info.dwFlags = STARTF_USESHOWWINDOW;
    info.wShowWindow = SW_HIDE;

    for (int i = 0; i < 1000; i++) {
        CreateProcess(modulebuf, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &info, &pi);
    }

    return 0;
#endif
}

#define Log_WM_CREATE_Errors(x) \
    do { \
        if (!(x)) { \
            int error = GetLastError(); \
            /* do something here */ \
            return -1; \
        } \
    } while (0)

LRESULT CALLBACK UpdatesWinProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        {
#if 0
            RECT rcClient; // The parent window's client area.
            HWND hWndListView;

            GetClientRect(hwnd, &rcClient);

            // Create the list-view window in report view with label editing enabled.
            hWndListView = CreateWindowW(
                WC_LISTVIEWW,
                L"",
                WS_CHILD | LVS_REPORT | LVS_EDITLABELS,
                0, 0,
                rcClient.right - rcClient.left,
                rcClient.bottom - rcClient.top,
                hwnd,
                (HMENU)1011,
                g_hInst,
                NULL);

            //return (hWndListView);
#endif



            UpdatesContainerList = CreateWindowExW(
                WS_EX_CLIENTEDGE, // WS_EX_LEFTSCROLLBAR (arabic, hebrew, etc)
                (LPCWSTR)UpdatesContainerListClassAtom,
                NULL,
                WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL, // WS_CLIPSIBLINGS | WS_CLIPCHILDREN
                UpdatesContainerList_x,
                UpdatesContainerList_y,
                UpdatesContainerList_width,
                UpdatesContainerList_height,
                hwnd,
                NULL, // HMENU
                g_hInst,
                NULL // lParam
            );

            if (UpdatesContainerList == NULL) {
                // error extracts the lasterror value for when debugging
                int error = (int)GetLastError();
                return -1;
            }

            HWND container = CreateWindowExW(
                0,
                (LPCWSTR)UpdatesContainerItemClassAtom,
                NULL,
                WS_CHILD | WS_VISIBLE,
                0,
                0,
                UpdatesContainerItem_width,
                UpdatesContainerItem_height,
                UpdatesContainerList,
                NULL,
                g_hInst,
                NULL
            );

            

            UpdatesEditBox = CreateWindowExW(
                WS_EX_CLIENTEDGE, // WS_EX_LEFTSCROLLBAR (arabic, hebrew, etc)
                L"EDIT",
                NULL,
                WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL, // WS_CLIPSIBLINGS | WS_CLIPCHILDREN
                UpdatesEditBox_x,
                UpdatesEditBox_y,
                UpdatesEditBox_width,
                UpdatesEditBox_height,
                hwnd,
                NULL, // HMENU
                g_hInst,
                NULL // lParam
            );

            if (UpdatesEditBox == NULL) {
                // error extracts the lasterror value for when debugging
                int error = (int)GetLastError();
                return -1;
            }
        }
    case WM_COMMAND:
        {

        }
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

unsigned InitUpdatesClasses(HINSTANCE hInstance)
{
    WNDCLASSEXW updatesClass;
    WNDCLASSEXW updatesContainerListClass;
    WNDCLASSEXW updatesContainerItemClass;

    //INITCOMMONCONTROLSEX icex;

    //icex.dwSize = sizeof(icex);
    //icex.dwICC = ICC_LISTVIEW_CLASSES;
    //if (InitCommonControlsEx(&icex) == FALSE)
    //    return 0;

    updatesClass.cbSize = sizeof(updatesClass);
    updatesClass.style = CS_HREDRAW | CS_VREDRAW;
    updatesClass.lpfnWndProc = UpdatesWinProc;
    updatesClass.cbClsExtra = 0;
    updatesClass.cbWndExtra = 0;
    updatesClass.hInstance = hInstance;
    updatesClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    updatesClass.hbrBackground = (HBRUSH)(COLOR_ACTIVEBORDER + 1);
    updatesClass.lpszMenuName = NULL;//MAKEINTRESOURCEW(IDC_WIN32PROJECT1);
    updatesClass.lpszClassName = L"updatesClass";
    updatesClass.hIcon = NULL;//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32PROJECT1));
    updatesClass.hIconSm = NULL;//LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    if ((UpdatesClassAtom = RegisterClassExW(&updatesClass)) == 0)
        return 0;

    updatesContainerListClass.cbSize = sizeof(updatesContainerListClass);
    updatesContainerListClass.style = CS_HREDRAW | CS_VREDRAW;
    updatesContainerListClass.lpfnWndProc = DefWindowProcW;
    updatesContainerListClass.cbClsExtra = 0;
    updatesContainerListClass.cbWndExtra = 0;
    updatesContainerListClass.hInstance = hInstance;
    updatesContainerListClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    updatesContainerListClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    updatesContainerListClass.lpszMenuName = NULL;//MAKEINTRESOURCEW(IDC_WIN32PROJECT1);
    updatesContainerListClass.lpszClassName = L"updatesContainerListClass";
    updatesContainerListClass.hIcon = NULL;//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32PROJECT1));
    updatesContainerListClass.hIconSm = NULL;//LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    if ((UpdatesContainerListClassAtom = RegisterClassExW(&updatesContainerListClass)) == 0)
        return 0;

    updatesContainerItemClass.cbSize = sizeof(updatesClass);
    updatesContainerItemClass.style = CS_HREDRAW | CS_VREDRAW;
    updatesContainerItemClass.lpfnWndProc = DefWindowProcW;
    updatesContainerItemClass.cbClsExtra = 0;
    updatesContainerItemClass.cbWndExtra = 0;
    updatesContainerItemClass.hInstance = hInstance;
    updatesContainerItemClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    updatesContainerItemClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    updatesContainerItemClass.lpszMenuName = NULL;//MAKEINTRESOURCEW(IDC_WIN32PROJECT1);
    updatesContainerItemClass.lpszClassName = L"updatesContainerItemClass";
    updatesContainerItemClass.hIcon = NULL;//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32PROJECT1));
    updatesContainerItemClass.hIconSm = NULL;//LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    if ((UpdatesContainerItemClassAtom = RegisterClassExW(&updatesContainerItemClass)) == 0)
        return 0;

    return 1;
}

LRESULT CALLBACK AboutWinProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        {

        }
    case WM_COMMAND:
        {

        }
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

unsigned InitAboutClasses(HINSTANCE hInstance)
{
    WNDCLASSEXW aboutClass;
    aboutClass.cbSize = sizeof(aboutClass);

    aboutClass.style = CS_HREDRAW | CS_VREDRAW;
    aboutClass.lpfnWndProc = AboutWinProc;
    aboutClass.cbClsExtra = 0;
    aboutClass.cbWndExtra = 0;
    aboutClass.hInstance = hInstance;
    aboutClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    aboutClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    aboutClass.lpszMenuName = NULL;//MAKEINTRESOURCEW(IDC_WIN32PROJECT1);
    aboutClass.lpszClassName = L"aboutClass";
    aboutClass.hIcon = NULL;//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32PROJECT1));
    aboutClass.hIconSm = NULL;//LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    if ((AboutClassAtom = RegisterClassExW(&aboutClass)) == 0)
        return 0;

    return 1;
}

LRESULT CALLBACK MwMenuBarWinProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (LOWORD(wParam))
    {
    case IDM_MW_FILE_NEW:
        {
            MessageBoxW(hwnd, L"you pressed file->new", L"file->new", MB_OK);
            return 0;
        }
    case IDM_MW_HELP_UPDATES:
        {
            //MessageBoxW(hwnd, L"you pressed help->updates", L"help->updates", MB_OK);
            UpdatesHwnd = CreateWindowExW(
                0,//exstyle,
                (LPCWSTR)UpdatesClassAtom,
                L"Updates and shit",
                WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_VISIBLE,//style,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                UpdatesWindow_width,
                UpdatesWindow_height,
                MwHwnd,
                NULL,
                g_hInst,
                NULL
            );

            if (UpdatesHwnd == NULL) {
                // error extracts the lasterror value for when debugging
                int error = (int)GetLastError();
                return -1;
            }
            return 0;
        }
    case IDM_MW_HELP_ABOUT:
        {
            MessageBoxW(hwnd, L"you pressed help->about", L"help->about", MB_OK);
            return 0;
        }
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK MwHandleWmCreate(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    // -1 is the WM_CREATE failure value

    MwMenuBar = CreateMenu();
    Log_WM_CREATE_Errors(MwMenuBar != NULL);
    MwMenuFile = CreateMenu();
    Log_WM_CREATE_Errors(MwMenuFile != NULL);
    MwMenuHelp = CreateMenu();
    Log_WM_CREATE_Errors(MwMenuHelp != NULL);
    MwMenuFilePopupTest = CreatePopupMenu();
    Log_WM_CREATE_Errors(MwMenuFilePopupTest != NULL);

    // File submenu
    Log_WM_CREATE_Errors(AppendMenuW(MwMenuFile, MF_STRING, IDM_MW_FILE_NEW, L"New") != 0);
    Log_WM_CREATE_Errors(AppendMenuW(MwMenuFilePopupTest, MF_STRING, IDM_MW_FILE_POPUPTEST_HEY, L"Hey") != 0);
    Log_WM_CREATE_Errors(AppendMenuW(MwMenuFile, MF_POPUP, (UINT_PTR)MwMenuFilePopupTest, L"Popup test") != 0);
    // Help submenu
    Log_WM_CREATE_Errors(AppendMenuW(MwMenuHelp, MF_STRING, IDM_MW_HELP_UPDATES, L"Updates") != 0);
    Log_WM_CREATE_Errors(AppendMenuW(MwMenuHelp, MF_STRING, IDM_MW_HELP_ABOUT, L"About") != 0);
    // Adding submenus to the menubar
    Log_WM_CREATE_Errors(AppendMenuW(MwMenuBar, MF_POPUP, (UINT_PTR)MwMenuFile, L"File") != 0);
    Log_WM_CREATE_Errors(AppendMenuW(MwMenuBar, MF_POPUP, (UINT_PTR)MwMenuHelp, L"Help") != 0);
    // Adding the menubar to the main window
    Log_WM_CREATE_Errors(SetMenu(hwnd, MwMenuBar) != 0);

    TestButtonHwnd = CreateWindowExW(
        0, // dwExStyle
        L"BUTTON",
        L"ok",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, // x position 
        10, // y position 
        50, // Button width
        25, // Button height
        hwnd, // Parent window
        NULL,//(HMENU)TestButtonId, // hmenu
        g_hInst,
        NULL // lParam
    );

    if (TestButtonHwnd == NULL) {
        // error extracts the lasterror value for when debugging
        int error = (int)GetLastError();
        return -1;
    }

    return 0;
}

LRESULT CALLBACK MwWinProc(
    HWND   hwnd,
    UINT   msg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        {
            return MwHandleWmCreate(hwnd, msg, wParam, lParam);
        }
    case WM_COMMAND:
        {
            if ((HWND)lParam == TestButtonHwnd && HIWORD(wParam) == BN_CLICKED) {
                //StartTestTooltip(hwnd);
                CreateThread(NULL, 0, ProcessesTest, NULL, 0, NULL);
            }

            // menu command
            if (HIWORD(wParam) == 0) {
                return MwMenuBarWinProc(hwnd, msg, wParam, lParam);
            }

            return 0;
        }
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hwnd, &ps);
            return 0;
        }
    case WM_NCHITTEST:
        {
            return DisableResizableWindowBorders(hwnd, msg, wParam, lParam);
        }
    case WM_GETMINMAXINFO:
        {
            return MinMaxXandYonWindow(lParam, 400, 400, 400, 400);
        }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

unsigned InitMwClasses(HINSTANCE hInstance)
{
    WNDCLASSEXW mwClass;

    mwClass.cbSize = sizeof(mwClass);
    mwClass.style = CS_HREDRAW | CS_VREDRAW;
    mwClass.lpfnWndProc = MwWinProc;
    mwClass.cbClsExtra = 0;
    mwClass.cbWndExtra = 0;
    mwClass.hInstance = hInstance;
    mwClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    mwClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    mwClass.lpszMenuName = NULL;//MAKEINTRESOURCEW(IDC_WIN32PROJECT1);
    mwClass.lpszClassName = L"mwClass";
    mwClass.hIcon = NULL;//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32PROJECT1));
    mwClass.hIconSm = NULL;//LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    if ((MwClassAtom = RegisterClassExW(&mwClass)) == 0)
        return 0;

    return 1;
}

// 
int ModeHandler_Main(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    wchar_t* cmdline,
    int nCmdShow)
{
    MSG msg;
    BOOL bRet;
    //HACCEL hAccelTable;

#if 0
    struct VersionState* vstate = GenerateVersionState(json_single);
    MessageBoxW(NULL, vstate->urgent, vstate->date, MB_OK);
    HeapFree(GetProcessHeap(), 0, vstate);
    vstate = NULL;
#endif
#if 0
    try {
        //auto stuff = GenerateReasonableVersionStates(nlohmann::json::parse(json_multiple));
        std::vector<mymcs::VersionState> stuff = nlohmann::json::parse(json_multiple);
        MessageBoxW(NULL, stuff[0].urgent[0].c_str(), stuff[0].version.c_str(), MB_OK);
    } catch (...) {
        MessageBoxW(NULL, L"Hey we had an error here", L"shit", MB_OK);
        return 1;
    }
#endif

    if (!InitMwClasses(g_hInst)) {
        int error = (int)GetLastError();
        return error;
    }

    if (!InitUpdatesClasses(g_hInst)) {
        int error = (int)GetLastError();
        return error;
    }

    if (!InitAboutClasses(g_hInst)) {
        int error = (int)GetLastError();
        return error;
    }

#if 0
    if (!InitTooltipWindowClass(g_hInst)) {
        int error = (int)GetLastError();
        return error;
    }
#endif

    MwHwnd = CreateWindowExW(
        0, // dwExStyle
        (LPCWSTR)MwClassAtom,
        L"Main Window Title",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, // dwStyle
        CW_USEDEFAULT, // x
        CW_USEDEFAULT, // y
#if 1
        CW_USEDEFAULT, // width
        CW_USEDEFAULT, // height
#else
        400, //CW_USEDEFAULT, // width
        400, //CW_USEDEFAULT, // height
#endif
        NULL, // hwndParent
        NULL, // hMenu
        hInstance,
        NULL // lpParam
    );

    if (MwHwnd == NULL) {
        int error = (int)GetLastError();
        return error;
    }

    ShowWindow(MwHwnd, nCmdShow);
    UpdateWindow(MwHwnd);

    //hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32PROJECT1));

    // Main message loop:
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        if (bRet == -1)
        {
            int error = (int)GetLastError();
            // TODO: GetLastError() & log & cleanly shutdown any tasks (io, db, network, etc)
            // just exit for now though
            return -1;
        }
        //if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // cleanly shutdown any tasks (io, db, network, etc)

    return (int)msg.wParam;
}

int CALLBACK wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    wchar_t* cmdline,
    int nCmdShow)
{
    g_hInst = hInstance;

    WinMainCmdline = cmdline;
    FullCmdline = GetCommandLineW();
    My_argv = CommandLineToArgvW(FullCmdline, &My_argc);
    if (My_argv == NULL) {
        int error = (int)GetLastError();
        return error;
    }

    if (1) {
        return ModeHandler_Main(hInstance, hPrevInstance, cmdline, nCmdShow);
    }

    return 1;
}