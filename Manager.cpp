#include "Manager.h"
#include <windef.h>

Container* root;

size_t getWindowsOnMonitor(MONITORINFO currentMonitor, std::vector<HWND>& windowsOnMonitor)
{ // fills vector with windows on monitor 0 on success
    MONITORINFO temp;
    temp.cbSize = sizeof(MONITORINFO);
    for(auto window : windows)
    {
        WINDOWPLACEMENT placement;
        placement.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(window, &placement);
        if(GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST), &temp))
        {
            if(temp.rcMonitor == currentMonitor.rcMonitor)
            {
                if(placement.showCmd == SW_SHOWMAXIMIZED) // Checks if window is maximized
                    return -1;
                windowsOnMonitor.push_back(window);
            }
        }
    }
    return 0;
}

void splitWindow(MONITORINFO currentMonitor, std::vector<HWND> windowsOnMonitor)
{
    std::cout << "Splitting window" << std::endl;
    int top;
    int left;
    int right;
    int bottom;
    WindowData newWindowData = windowMap[windowsOnMonitor[0]];

    if(windowsOnMonitor.size() == 1)
    {
        top = currentMonitor.rcMonitor.top + borderGap;
        left = currentMonitor.rcMonitor.left + borderGap;
        right = currentMonitor.rcMonitor.right - borderGap;
        bottom = currentMonitor.rcMonitor.bottom - borderGap - taskBarHeight;
        MoveWindow(newWindowData.hwnd_, left, top, right - left, bottom - top, TRUE);
        return;
    }

    WindowData activeWindowData = windowMap[windowsOnMonitor[1]];
    top = activeWindowData.rect_.top;
    left = activeWindowData.rect_.left;
    right = activeWindowData.rect_.right;
    bottom = activeWindowData.rect_.bottom;

    int resolutionX = right - left;
    int resolutionY = bottom - top;

    WindowData* aWindowData = &windowMap[activeWindowData.hwnd_];
    WindowData* nWindowData = &windowMap[newWindowData.hwnd_];

    aWindowData->rectBeforeSplit_ = activeWindowData.rect_;
    nWindowData->rectBeforeSplit_ = activeWindowData.rect_;

    aWindowData->nextWindow_ = nWindowData;
    nWindowData->previousWindow_ = aWindowData;

    Container* newParent = new Container();
    newParent->addLeaf(aWindowData);

    if(aWindowData->parent_)
    {
        aWindowData->parent_->removeLeaf(aWindowData);
        aWindowData->parent_->addLeaf(newParent);
        aWindowData->parent_ = newParent;
    }
    else
    {
        root = newParent;
    }
    newParent->rect_ = activeWindowData.rect_;
    aWindowData->parent_ = newParent;

    if(activeWindowData.formatDirection_ == FormatDirection::VERTICAL)
    {
        MoveWindow(activeWindowData.hwnd_, left, top, resolutionX / 2, resolutionY, TRUE);
        MoveWindow(newWindowData.hwnd_, left + resolutionX / 2, top, resolutionX / 2, resolutionY, TRUE);
    }
    else if(activeWindowData.formatDirection_ == FormatDirection::HORIZONTAL)
    {
        MoveWindow(activeWindowData.hwnd_, left, top, resolutionX, resolutionY / 2, TRUE);
        MoveWindow(newWindowData.hwnd_, left, top + resolutionY / 2, resolutionX, resolutionY / 2, TRUE);
    }
    toggleFormatDirection(activeWindowData.hwnd_);
    nWindowData->formatDirection_ = aWindowData->formatDirection_;

    nWindowData->parent_ = aWindowData->parent_;
    newParent->addLeaf(nWindowData);

    std::cout << std::endl << std::endl;
    root->printStructure();
    return;
}

void resetWindowPosition(HWND hwnd)
{
    WindowData windowData = windowMap[hwnd];
    MoveWindow(hwnd,
               windowData.rectBeforeSplit_.left,
               windowData.rectBeforeSplit_.top,
               windowData.rectBeforeSplit_.right - windowData.rectBeforeSplit_.left,
               windowData.rectBeforeSplit_.bottom - windowData.rectBeforeSplit_.top,
               TRUE);
    return;
}

void onWindowCountChanged(MONITORINFO currentMonitor, std::vector<HWND> windowsOnMonitor)
{
    // Format windows
    if(prevAmountOfWindows < amountOfWindows)
        splitWindow(currentMonitor, windowsOnMonitor);
    else if(prevAmountOfWindows > amountOfWindows)
        resetWindows();

    return;
}

void updateWindows(bool windowCountChanged, bool windowPositionChanged)
{
    // Get current monitor
    MONITORINFO currentMonitor;
    HWND window = windows.front();
    currentMonitor.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST), &currentMonitor);

    // Get windows on monitor
    std::vector<HWND> windowsOnMonitor = {};
    if(getWindowsOnMonitor(currentMonitor, windowsOnMonitor) == -1) // Fails if there is a maximized window (intended as when a window is maximized, it should not be formatted)
        return;

    if(windowCountChanged)
        onWindowCountChanged(currentMonitor, windowsOnMonitor);
    if(windowPositionChanged)
        letWindowsFillSpace(currentMonitor.rcMonitor, windowsOnMonitor);
    return;
}

void letWindowsFillSpace(RECT space, std::vector<HWND> windows)
{
    // TODO
    return;
}

void MoveWindowToRect(HWND hwnd, RECT rect)
{
    MoveWindow(hwnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
}

void resetWindows()
{
    std::cout << "Resetting windows" << std::endl;
    if(root)
      root->printStructure();
    std::map<HWND, WindowData> windowMapCopy(windowMap);
    for(auto window : windowMapCopy)
    {
        if(!doesWindowExist(window.first)) // then window.first got closed
        {
            std::cout << "Window closed" << std::endl;
            WindowData closedWindow = window.second;
            Container* parent = closedWindow.parent_;
            windowMap.erase(window.first);
            if(parent == nullptr)
                return;

            // Case 1: The parent only had leafs
            if(parent->getWindowCount() == 2)
            {
                std::cout << "Case 1" << std::endl;
                for(auto leaf : parent->m_leafs_)
                {
                    WindowData* leafData = dynamic_cast<WindowData*>(leaf);
                    parent->removeLeaf(leafData);
                    if(leafData->id_ == closedWindow.id_)
                        continue;

                    std::cout << "Leaf: " << leafData->title_ << std::endl;

                    leafData->rect_ = parent->rect_;
                    MoveWindowToRect(leafData->hwnd_, parent->rect_);
                    Container* grandParent = parent->m_parent_;
                    if(grandParent == nullptr)
                    {
                        std::cout << "Grandparent is nullptr" << std::endl;
                        leafData->parent_ = nullptr;
                    }
                    else
                    {
                        std::cout << "Grandparent is not nullptr" << std::endl;
                        grandParent->addLeaf(leafData);
                        std::cout << "Added leaf to grandparent" << std::endl;
                        grandParent->removeLeaf(parent);
                        std::cout << "Removed parent from grandparent" << std::endl;
                        leafData->parent_ = grandParent;
                        std::cout << "Set leaf parent to grandparent" << std::endl;
                    }
                    std::cout << "Finished leaf" << std::endl;
                }
                std::cout << "Deleting parent" << std::endl;
                delete parent;
            }
        }
    }
}
