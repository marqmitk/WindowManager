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
    WindowData& nWindowData = windowMap[windowsOnMonitor[0]];

    if(windowsOnMonitor.size() == 1)
    {
        top = currentMonitor.rcMonitor.top + borderGap;
        left = currentMonitor.rcMonitor.left + borderGap;
        right = currentMonitor.rcMonitor.right - borderGap;
        bottom = currentMonitor.rcMonitor.bottom - borderGap - taskBarHeight;
        MoveWindow(nWindowData.hwnd_, left, top, right - left, bottom - top, TRUE);
        return;
    }

    WindowData& aWindowData = windowMap[windowsOnMonitor[1]];

    top = aWindowData.rect_.top;
    left = aWindowData.rect_.left;
    right = aWindowData.rect_.right;
    bottom = aWindowData.rect_.bottom;

    int resolutionX = right - left;
    int resolutionY = bottom - top;

    Container* newParent = new Container();
    if(aWindowData.parent_)
    {
        aWindowData.parent_->removeLeaf(&aWindowData);
        aWindowData.parent_->addLeaf(newParent);
        newParent->parent_ = aWindowData.parent_;
        aWindowData.parent_ = newParent;
    }
    else
    {
        root = newParent;
    }
    newParent->rect_ = aWindowData.rect_;
    newParent->addLeaf(&nWindowData);
    newParent->addLeaf(&aWindowData);
    aWindowData.parent_ = newParent;
    nWindowData.parent_ = newParent;

    if(aWindowData.formatDirection_ == FormatDirection::VERTICAL)
    {
        MoveWindow(aWindowData.hwnd_, left, top, resolutionX / 2, resolutionY, TRUE);
        MoveWindow(nWindowData.hwnd_, left + resolutionX / 2, top, resolutionX / 2, resolutionY, TRUE);
    }
    else if(aWindowData.formatDirection_ == FormatDirection::HORIZONTAL)
    {
        MoveWindow(aWindowData.hwnd_, left, top, resolutionX, resolutionY / 2, TRUE);
        MoveWindow(nWindowData.hwnd_, left, top + resolutionY / 2, resolutionX, resolutionY / 2, TRUE);
    }

    aWindowData.toggleFormatDirection();
    nWindowData.formatDirection_ = aWindowData.formatDirection_;
    newParent->formatDirection_ = aWindowData.formatDirection_;

    std::cout << "Root is: " << root->id_ << std::endl;
    root->printStructure();
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
    // if(windowPositionChanged)

    return;
}

void resetWindows()
{
    std::cout << "Resetting windows" << std::endl;
    std::cout << "Root is: " << root->id_ << std::endl;
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
            {
                std::cout << "Parent is nullptr" << std::endl;
                return;
            }

            parent->removeLeaf(&closedWindow);

            // Case 1: The parent only had leafs
            if(parent->getWindowCount() == 2)
            {
                std::cout << "Case 1" << std::endl;
                for(auto leaf : parent->m_leafs_)
                {
                    WindowData* leafData = dynamic_cast<WindowData*>(leaf);
                    if(leafData->id_ == closedWindow.id_)
                        continue;

                    leafData->rect_ = parent->rect_;
                    MoveWindowToRect(leafData->hwnd_, parent->rect_);
                }
            }
            // Case 2: The parent had a leaf and a container
            else if(parent->getWindowCount() == 1)
            {
                std::cout << "Case 2" << std::endl;
                RECT rectToFill = parent->rect_;
                Container* otherContainer = nullptr;
                for(auto leaf : parent->m_leafs_)
                {
                    if(leaf->type_ == DesktopType::CONTAINER)
                    {
                        otherContainer = dynamic_cast<Container*>(leaf);
                        break;
                    }
                }

                if(root == parent)
                    root = otherContainer;

                windowMap.erase(closedWindow.hwnd_);
                otherContainer->sizeUp(rectToFill);
                otherContainer->updateWindowPositions();
                otherContainer->toggleFormatDirection();

            }
            else
            {
                std::cout << "Case 3: Something went wrong" << std::endl;
                std::cout << "We have " << parent->getWindowCount() << " windows in the parent" << std::endl;
                exit(1);
            }
            Container* grandParent = parent->parent_;

            auto leafsCopy = parent->m_leafs_;

            for(auto leaf : leafsCopy)
            {
                Desktop* leafData = dynamic_cast<Desktop*>(leaf);
                parent->removeLeaf(leafData);

                if(leafData->type_ == DesktopType::WINDOW)
                    if(dynamic_cast<WindowData*>(leafData)->id_ == closedWindow.id_)
                        continue;

                if(grandParent == nullptr)
                {
                    std::cout << "Grandparent is nullptr" << std::endl;
                    leafData->parent_ = nullptr;
                }
                else
                {
                    std::cout << "Grandparent is not nullptr" << std::endl;
                    grandParent->addLeaf(leafData);
                    grandParent->removeLeaf(parent);
                    leafData->parent_ = grandParent;
                    leafData->parent_->toggleFormatDirection();
                }
                leafData->toggleFormatDirection();
            }
            delete parent;
        }
    }

    std::cout << "Root is: " << root->id_ << std::endl;
    if(root)
        root->printStructure();
}
