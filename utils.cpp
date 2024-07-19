#include "utils.h"

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
    "▄berlauffenster der Taskleiste.",
    "PowerToys Find My Mouse",
    "Ausf³hren",
};

int taskBarHeight;

bool operator==(const RECT& lhs, const RECT& rhs)
{ // Compares two RECT structs
  return lhs.left == rhs.left && lhs.right == rhs.right && lhs.top == rhs.top && lhs.bottom == rhs.bottom;
}

BOOL CALLBACK saveWindow(HWND hwnd, LPARAM substring)
{

  const DWORD TITLE_SIZE = 1024;
  TCHAR windowTitle[TITLE_SIZE];

  GetWindowText(hwnd, windowTitle, TITLE_SIZE);
  int length = ::GetWindowTextLength(hwnd);

  std::string temp(&windowTitle[0]);
  std::string title(temp.begin(), temp.end());

  if(!IsWindowVisible(hwnd) || length == 0)
    return TRUE;

  if(std::find(blacklist.begin(), blacklist.end(), title) != blacklist.end())
    return TRUE;

  windowMap[hwnd].title_ = title;
  windowMap[hwnd].hwnd_ = hwnd;
  // get coordinates
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

void toggleFormatDirection(HWND hwnd)
{
  if(windowMap[hwnd].formatDirection_ == FormatDirection::HORIZONTAL)
    windowMap[hwnd].formatDirection_ = FormatDirection::VERTICAL;
  else
    windowMap[hwnd].formatDirection_ = FormatDirection::HORIZONTAL;
  return;
}
