#include "utils.h"
#include <iostream>
#include <math.h>
#include <string>
#include <tchar.h>
#include <vector>
#include <windows.h>
#include <winuser.h>

extern Container* root;

void onWindowCountChanged(MONITORINFO currentMonitor, std::vector<HWND> windowsOnMonitor);
void updateWindows(bool windowCountChanged, bool windowPositionChanged);
void splitWindow(MONITORINFO currentMonitor, std::vector<WindowData> windowsOnMonitor);
void resetWindows();
size_t getWindowsOnMonitor(MONITORINFO currentMonitor, std::vector<WindowData>& windowsOnMonitor);
