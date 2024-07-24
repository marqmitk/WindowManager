#include "Manager.hpp"
#include <thread>
#include <winuser.h>

BOOL running = true;

int main()
{
    updateWindowContainers();
    std::cout << "Ready" << std::endl;

    std::thread t1(drawBorders);

    while(running)
    {

        listenForKeybinds();
        updateWindowContainers();

        if(prevAmountOfWindows != amountOfWindows)
            updateWindows(true);

        handleWindowMovement();
        handleWindowResize();

        prevAmountOfWindows = amountOfWindows;
    }
    // kill thread
    std::terminate();
}
