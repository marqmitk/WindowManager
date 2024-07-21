#include "Manager.hpp"
#include <winuser.h>

BOOL running = true;

int main()
{
    std::cout << "Ready" << std::endl;

    updateWindowContainers();

    while(running)
    {
        listenForKeybinds();
        updateWindowContainers();

        if(prevAmountOfWindows != amountOfWindows)
            updateWindows(true);

        HWND windowGettingMoved = getWindowGettingMoved();
        if(windowGettingMoved != lastWindowGettingMoved && lastWindowGettingMoved != nullptr)
        {
            updateWindows();
            lastWindowGettingMoved = windowGettingMoved;
        } else if(windowGettingMoved != nullptr && lastWindowGettingMoved == nullptr)
        {
            lastWindowGettingMoved = windowGettingMoved;
            windowMap[lastWindowGettingMoved].previousRect_ = windowMap[lastWindowGettingMoved].getOriginalRect();
        }

        prevAmountOfWindows = amountOfWindows;
    }
    return 0;
}
