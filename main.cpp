#include "Manager.hpp"
#include <winuser.h>

BOOL running = true;

int main()
{

    taskBarHeight = GetTaskBarHeight();
    std::cout << "Taskbar height: " << taskBarHeight << std::endl;

    EnumWindows(saveWindow, 0);
    amountOfWindows = windows.size();
    prevAmountOfWindows = amountOfWindows;
    std::cout << "Ready" << std::endl;

    while(running)
    {
        if(GetAsyncKeyState(VK_F3) & 0x8000) // for debugging purposes
            exit(0);

        if(GetAsyncKeyState(VK_F2) & 0x8000) // for debugging purposes
            system("start cmd");

        windows.clear();
        amountOfWindows = 0;
        EnumWindows(saveWindow, 0);

        bool windowCountChanged = prevAmountOfWindows != amountOfWindows;
        bool windowPositionChanged = DidWindowPositionChange();
        windowPositionChanged = false; // for debugging purposes

        if(windowCountChanged || windowPositionChanged)
            updateWindows(windowCountChanged, windowPositionChanged);

        prevAmountOfWindows = amountOfWindows;
    }

    return 0;
}
