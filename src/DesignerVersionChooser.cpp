// DesignerVersionChooser.cpp : Defines the entry point for the application.
//

#include <windowsx.h>
#include "framework.h"
#include "shellapi.h"
#include "DesignerVersionChooser.h"
#include "DesignerVersionList.h"

#define MAX_LOADSTRING 100

// Enable debugging console output
// #define DEBUG 1

// Context menu options
#define IDM_CONTEXT_DEVMODE     1000
#define IDM_CONTEXT_UNINSTALL   1001
#define IDM_CONTEXT_REVEAL      1002

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND  hwList;                                   // The list box
WCHAR szCommandLine[MAX_PATH];                  // Copy of the command line parameters

DesignerVersionList mList;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    MessageHandler(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);

    wcscpy(szCommandLine, lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DESIGNERVERSIONCHOOSER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

#ifdef _DEBUG
    // Shows debugging console for stdout
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
#endif

    DWORD ticksStart = GetTickCount();
    mList.doSearch();
    DWORD ticksEnd = GetTickCount();

    printf("Seach took %d ms", ticksEnd - ticksStart);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    return 0;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    //wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DESIGNERVERSIONCHOOSER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DESIGNERVERSIONCHOOSER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, MessageHandler);

   return TRUE;
}

VOID LaunchDesigner(bool debugMode)
{
    int selItem = (INT)SendMessage(hwList, LB_GETCURSEL, 0, 0);
    std::wstring path = mList.at(selItem).executablePath();
    std::wstring directory = mList.at(selItem).directoryPath();
    std::wstring commandLine;
    if(debugMode)
    {
        commandLine += std::wstring(L"-d ");
    }
    commandLine.append(szCommandLine);
    ShellExecute(NULL, NULL, path.c_str(), commandLine.data(), directory.c_str(), SW_NORMAL);
}


VOID UninstallDesigner()
{
    int selItem = (INT)SendMessage(hwList, LB_GETCURSEL, 0, 0);
    std::wstring path = mList.at(selItem).uninstallerPath();
    ShellExecute(NULL, NULL, path.c_str(), NULL, NULL, SW_NORMAL);
}


VOID ShowInExplorer()
{
    int selItem = (INT)SendMessage(hwList, LB_GETCURSEL, 0, 0);
    std::wstring path = mList.at(selItem).directoryPath();
    ShellExecute(NULL, L"explore", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

// Message handler for about box.
INT_PTR CALLBACK MessageHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        // Add items to list.
        hwList = GetDlgItem(hDlg, IDC_LIST1);

        int index = 0;
        for (auto i : mList)
        {
            int pos = (int)SendMessage(hwList, LB_ADDSTRING, 0,
                (LPARAM)i.toString().c_str());
            // The item data is the position in the map
            SendMessage(hwList, LB_SETITEMDATA, pos, (LPARAM)index);
        }
        // Set input focus to the list box.
        SetFocus(hwList);

        // Select the first (newest) version
        SendMessage(hwList, LB_SETSEL, TRUE, 0);

        // Center the window on the screen
        int xWidth = GetSystemMetrics(SM_CXSCREEN);
        int yWidth = GetSystemMetrics(SM_CYSCREEN);
        RECT rect;
        GetWindowRect(hDlg, &rect);
        SetWindowPos(hDlg, NULL,
            xWidth / 2 - (rect.right - rect.left) / 2,
            yWidth / 2 - (rect.bottom - rect.top) / 2,
            -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        return FALSE; // False ensures focus maintained on the listbox
    }


    case WM_COMMAND:
    {
        if (LOWORD(wParam) == IDOK)
        {
            LaunchDesigner(false);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        if (HIWORD(wParam) == LBN_DBLCLK ||
            (HIWORD(wParam) == 0 && LOWORD(wParam) == IDM_CONTEXT_DEVMODE))
        {
            auto state = GetKeyState(VK_CONTROL);
            bool debug = (state == -127) || (HIWORD(wParam) == 0 && LOWORD(wParam) == IDM_CONTEXT_DEVMODE);
            LaunchDesigner(debug);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        if (HIWORD(wParam) == 0 && LOWORD(wParam) == IDM_CONTEXT_UNINSTALL)
        {
            UninstallDesigner();
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        if (HIWORD(wParam) == 0 && LOWORD(wParam) == IDM_CONTEXT_REVEAL)
        {
            ShowInExplorer();
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
        break;

    case WM_VKEYTOITEM:
    {
        INT selItem = (INT)SendMessage(hwList, LB_GETCURSEL, 0, 0);
        INT count = (INT)SendMessage(hwList, LB_GETCOUNT, 0, 0);
        if (LOWORD(wParam) == VK_UP)
        {
            // Up key pressed
            selItem--;
            if (selItem < 0)
            {
                selItem = count - 1;
            }
            SendMessage(hwList, LB_SETCURSEL, (WPARAM)selItem, 0);
            return -2;
        }
        if (LOWORD(wParam) == VK_DOWN)
        {
            // Down key pressed
            selItem++;
            if (selItem > count - 1)
            {
                selItem = 0;
            }
            SendMessage(hwList, LB_SETCURSEL, (WPARAM)selItem, 0);
            return -2;
        }
        return -1;
    }

    case WM_CONTEXTMENU:
    {
        INT selItem = (INT)SendMessage(hwList, LB_GETCURSEL, 0, 0);
        if(selItem < 0)
        {
            return FALSE;
        }

        HMENU hPopupMenu = CreatePopupMenu();

        auto xPos = GET_X_LPARAM(lParam);
        auto yPos = GET_Y_LPARAM(lParam);
        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, IDM_CONTEXT_DEVMODE,   L"Developer Mode");
        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, IDM_CONTEXT_UNINSTALL, L"Uninstall");
        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, IDM_CONTEXT_REVEAL,    L"Reveal in Explorer");
        TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, xPos, yPos, 0, hDlg, NULL);
        return TRUE;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return TRUE;
    }
    return (INT_PTR)FALSE;
}
