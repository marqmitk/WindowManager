#pragma once
#include "Config.hpp"
#include "Containers.hpp"
#include <map>
#include <string>
#include <vector>
#include <windows.h>


class Container;
class WindowData;

extern int taskBarHeight;
extern std::vector<HWND> windows;
extern std::vector<Container*> containers;
extern std::map<HWND, WindowData> windowMap;
extern std::vector<std::string> blacklist;
extern int amountOfWindows;
extern int prevAmountOfWindows;
extern HWND lastWindowGettingMoved;
extern HWND lastWindowGettingResized;

bool operator==(const RECT& lhs, const RECT& rhs);

void drawBorders();
void listenForKeybinds();
void updateWindowContainers();
BOOL CALLBACK saveWindow(HWND hwnd, LPARAM substring);
int GetTaskBarHeight();
bool doesWindowExist(HWND hwnd);
bool isWindowSaved(HWND hwnd);
HWND getWindowGettingMoved();