#include "Manager.hpp"
#include <winuser.h>
#include <thread>

BOOL running = true;


int main()
{
    std::cout << "Ready" << std::endl;

    updateWindowContainers();

    std::thread t1(drawBorders);

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
    // kill thread
    std::terminate();



}
