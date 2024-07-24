#include "Utils.hpp"
#include <thread>
#include <windef.h>
#include <wingdi.h>

int amountOfWindows = 0;
int prevAmountOfWindows = 0;
HWND lastWindowGettingMoved = nullptr;
HWND lastWindowGettingResized = nullptr;

std::vector<HWND> windows;
std::vector<Container*> containers;
std::map<HWND, WindowData> windowMap;
std::vector<std::string> blacklist = {
    "Windows-Eingabeerfahrung",
    "NVIDIA GeForce Overlay",
    "Program Manager",
    "Programmumschaltung",
    "PopupHost",
    "Andockhilfe",
    "WinUI Desktop",
    "PowerToys.MeasureToolOverlay",
    "Überlauffenster der Taskleiste.",
    "PowerToys Find My Mouse",
    "Ausführen",
    "Einstellungen", // not happy with having that in the blacklist but it randomly pops up without settings being open
    "SystemResourceNotifyWindow",
};

int taskBarHeight = GetTaskBarHeight();

bool operator==(const RECT& lhs, const RECT& rhs)
{ // Compares two RECT structs
    return lhs.left == rhs.left && lhs.right == rhs.right && lhs.top == rhs.top && lhs.bottom == rhs.bottom;
}

void drawBorders()
{
    while(false)
    {
        std::cout << "Amount of windows: " << amountOfWindows << std::endl;
        for(auto hwnd : windows)
        {
            std::cout << "Window: " << windowMap[hwnd].title_ << std::endl;
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            HPEN pen = CreatePen(PS_SOLID | PS_INSIDEFRAME, 8, RGB(255, 0, 0));
            HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));

            SelectObject(hdc, pen);
            SelectObject(hdc, brush);

            RECT rect = windowMap[hwnd].rect_;

            Rectangle(hdc, 0, 0, rect.right, rect.bottom);
            Rectangle(hdc, 50, 50, 100, 100);

            DeleteObject(pen);
            DeleteObject(brush);
            Sleep(100);
        }
    }
}

void identifyWindows()
{
    int drawTime = 100;
    bool printed = false;
    HWND hdc = GetDesktopWindow();
    HDC hdcMem = GetDCEx(hdc, NULL, DCX_WINDOW | DCX_CACHE | DCX_LOCKWINDOWUPDATE);
    HPEN pen = CreatePen(PS_SOLID | PS_INSIDEFRAME, 24, RGB(0, 255, 0));
    HBRUSH brush = CreateSolidBrush(RGB(0, 255, 0));
    while(drawTime > 0)
    {
        for(auto hwnd : windows)
        {
            std::pair<int, int> position = {windowMap[hwnd].rect_.left + windowMap[hwnd].getWidth() / 2, windowMap[hwnd].rect_.top + windowMap[hwnd].getHeight() / 2};
            // draw id in the middle of the window
            int id = windowMap[hwnd].id_;
            // get desktop context
            // create font
            HFONT hFont = CreateFont(40, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"));
            HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);
            // draw text
            SetTextColor(hdcMem, RGB(255, 0, 0));
            SetBkMode(hdcMem, TRANSPARENT);
            TextOut(hdcMem, position.first, position.second, std::to_string(id).c_str(), std::to_string(id).size());

            RECT rect = windowMap[hwnd].rect_;
            SelectObject(hdcMem, pen);
            FrameRect(hdcMem, &rect, brush);
        }
        for(auto container : containers)
        {
            std::pair<int, int> position = {container->rect_.left + container->getWidth() / 2, container->rect_.top + container->getHeight() / 2};
            // draw id in the middle of the window
            std::string id = container->id_;
            // get desktop context
            HWND hdc = GetDesktopWindow();
            HDC hdcMem = GetDCEx(hdc, NULL, DCX_WINDOW | DCX_CACHE | DCX_LOCKWINDOWUPDATE);
            // create font
            HFONT hFont = CreateFont(40, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"));
            HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);
            // draw text
            SetTextColor(hdcMem, RGB(0, 255, 0));
            SetBkMode(hdcMem, TRANSPARENT);
            TextOut(hdcMem, position.first, position.second, id.c_str(), id.size());

            // draw border
            RECT rect = container->rect_;
            SelectObject(hdcMem, pen);
            FrameRect(hdcMem, &rect, brush);
            if(!printed)
                std::cout << "Container: " << id << " address: " << container << std::endl;
        }
        drawTime -= 1;
        printed = true;
        Sleep(1);
    }
    // cleanup
    DeleteObject(pen);
    DeleteObject(brush);
    ReleaseDC(hdc, hdcMem);
}

void listenForKeybinds()
{
    if(GetAsyncKeyState(VK_F3) & 0x8000) // for debugging purposes
        exit(0);

    if(GetAsyncKeyState(VK_F2) & 0x8000) // for debugging purposes
        system("start cmd");

    if(GetAsyncKeyState(VK_F6) & 0x8000) // for debugging purposes
        identifyWindows();

    Sleep(50);
}

void updateWindowContainers()
{
    windows.clear();
    amountOfWindows = 0;
    EnumWindows(saveWindow, 0);
}

BOOL CALLBACK saveWindow(HWND hwnd, LPARAM substring)
{

    const DWORD titleSize = 1024;
    TCHAR windowTitle[titleSize];

    GetWindowText(hwnd, windowTitle, titleSize);
    int length = ::GetWindowTextLength(hwnd);

    std::string temp(&windowTitle[0]);
    std::string title(temp.begin(), temp.end());

    if(!IsWindowVisible(hwnd) || length == 0 || hwnd == 0)
        return TRUE;

    for(auto blacklisted : blacklist)
        if(strncmp(title.c_str(), blacklisted.c_str(), blacklisted.size()) == 0)
            return TRUE;

    bool isSaved = isWindowSaved(hwnd);

    if(!isSaved)
    {
        windowMap[hwnd].id_ = WindowData::lastId++;
        windowMap[hwnd].type_ = DesktopType::WINDOW;
        windowMap[hwnd].title_ = title;
        windowMap[hwnd].hwnd_ = hwnd;
        windowMap[hwnd].neighbours_ = new Neighbours();
    }

    GetWindowRect(hwnd, &windowMap[hwnd].rect_);

    windows.push_back(hwnd);
    amountOfWindows++;

    return TRUE;
}

int GetTaskBarHeight()
{
    HWND taskBar = FindWindowEx(0, 0, "Shell_TrayWnd", 0);
    RECT taskBarRect;
    GetWindowRect(taskBar, &taskBarRect);
    return taskBarRect.bottom - taskBarRect.top;
}

bool doesWindowExist(HWND hwnd)
{
    if(hwnd == nullptr)
        return false;
    return std::find(windows.begin(), windows.end(), hwnd) != windows.end();
}

bool isWindowSaved(HWND hwnd)
{
    return windowMap.find(hwnd) != windowMap.end();
}

HWND getWindowGettingMoved()
{
    GUITHREADINFO guiInfo;
    guiInfo.cbSize = sizeof(GUITHREADINFO);

    GetGUIThreadInfo(GetWindowThreadProcessId(GetForegroundWindow(), NULL), &guiInfo);
    return guiInfo.hwndMoveSize;
}