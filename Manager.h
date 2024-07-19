#include "utils.h"
#include <iostream>
#include <math.h>
#include <string>
#include <tchar.h>
#include <vector>
#include <windows.h>
#include <winuser.h>

void onWindowCountChanged();
void splitWindow(MONITORINFO currentMonitor, std::vector<WindowData> windowsOnMonitor);
void resetWindowPosition(HWND hwnd);
void resetWindows();
void letWindowsFillSpace(RECT space, std::vector<HWND>);
size_t getWindowsOnMonitor(MONITORINFO currentMonitor, std::vector<WindowData> &windowsOnMonitor);
