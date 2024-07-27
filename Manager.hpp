#include "Utils.hpp"
#include <math.h>
#include <tchar.h>
#include <vector>
#include <windows.h>
#include <winuser.h>

extern Container* root;

void onWindowCountChanged(MONITORINFO currentMonitor, std::vector<HWND> windowsOnMonitor);

void handleWindowMovement();
void onWindowMoved(MONITORINFO currentMonitor, std::vector<HWND> windowsOnMonitor);

void handleWindowResize();

void updateNeighbours();

void updateWindows(bool windowCountChanged = false);
void splitWindow(MONITORINFO currentMonitor, std::vector<WindowData> windowsOnMonitor);
void resetWindows();
size_t getWindowsOnMonitor(MONITORINFO currentMonitor, std::vector<WindowData>& windowsOnMonitor);
WindowData* getOverlappedWindowData(WindowData* movedWindowData, std::vector<HWND> windowsOnMonitor);
void swapWindows(WindowData* window1, WindowData* window2);
