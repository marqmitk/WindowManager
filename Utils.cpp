#include "Utils.hpp"
#include <iostream>
#include <ostream>

int amountOfWindows = 0;
int prevAmountOfWindows = 0;

int gap = 3;
int borderGap = 3;

std::vector<HWND> windows;
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
};

int taskBarHeight;

bool operator==(const RECT& lhs, const RECT& rhs)
{ // Compares two RECT structs
    return lhs.left == rhs.left && lhs.right == rhs.right && lhs.top == rhs.top && lhs.bottom == rhs.bottom;
}

BOOL CALLBACK saveWindow(HWND hwnd, LPARAM substring)
{

    const DWORD titleSize = 1024;
    TCHAR windowTitle[titleSize];

    GetWindowText(hwnd, windowTitle, titleSize);
    int length = ::GetWindowTextLength(hwnd);

    std::string temp(&windowTitle[0]);
    std::string title(temp.begin(), temp.end());

    if(!IsWindowVisible(hwnd) || length == 0)
        return TRUE;

    for(auto blacklisted : blacklist)
        if(strncmp(title.c_str(), blacklisted.c_str(), blacklisted.size()) == 0)
            return TRUE;

    windowMap[hwnd].title_ = title;
    windowMap[hwnd].hwnd_ = hwnd;
    // get coordinates
    GetWindowRect(hwnd, &windowMap[hwnd].rect_);
    windowMap[hwnd].id_ = WindowData::lastId++;
    windowMap[hwnd].type_ = DesktopType::WINDOW;

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
    return std::find(windows.begin(), windows.end(), hwnd) != windows.end();
}

bool DidWindowPositionChange()
{
    for(auto windowData : windowMap)
        if(!(windowData.second.rect_ == windowData.second.previousRect_))
            return true;

    return false;
}
