#include "Utils.hpp"

int amountOfWindows = 0;
int prevAmountOfWindows = 0;
HWND lastWindowGettingMoved = nullptr;

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
    while(true)
    {
      // under construction
    }
}
void listenForKeybinds()
{
    if(GetAsyncKeyState(VK_F3) & 0x8000) // for debugging purposes
        exit(0);

    if(GetAsyncKeyState(VK_F2) & 0x8000) // for debugging purposes
        system("start cmd");
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

    if(!IsWindowVisible(hwnd) || length == 0)
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
