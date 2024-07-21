#include "Manager.hpp"
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
        top = currentMonitor.rcMonitor.top + 2 * gapSize;
        left = currentMonitor.rcMonitor.left + gapSize;
        right = currentMonitor.rcMonitor.right - gapSize;
        bottom = currentMonitor.rcMonitor.bottom - (gapSize / 2) - taskBarHeight;
        RECT rect = {left, top, right - left, bottom - top};
        nWindowData.moveWindowToRect(rect);
        return;
    }

    WindowData& aWindowData = windowMap[windowsOnMonitor[1]];

    aWindowData.rect_ = aWindowData.getOriginalRect();
    nWindowData.rect_ = nWindowData.getOriginalRect();
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

    RECT aRect;
    RECT nRect;

    if(aWindowData.formatDirection_ == FormatDirection::VERTICAL)
    {
        aRect = {left, top, left + resolutionX / 2, top + resolutionY};
        nRect = {left + resolutionX / 2, top, right, bottom};
    }
    else if(aWindowData.formatDirection_ == FormatDirection::HORIZONTAL)
    {
        aRect = {left, top, right, top + resolutionY / 2};
        nRect = {left, top + resolutionY / 2, right, bottom};
    }

    aWindowData.moveWindowToRect(aRect, gapSize);
    nWindowData.moveWindowToRect(nRect, gapSize);

    aWindowData.toggleFormatDirection();
    nWindowData.formatDirection_ = aWindowData.formatDirection_;
    newParent->formatDirection_ = aWindowData.formatDirection_;

    if(root)
    {
        std::cout << "Root is: " << root->id_ << std::endl;
        root->printStructure();
    }
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

void onWindowMoved(MONITORINFO currentMonitor, std::vector<HWND> windowsOnMonitor)
{
    WindowData& movedWindowData = windowMap[lastWindowGettingMoved];
    WindowData& overlappedWindowData = getOverlappedWindowData(movedWindowData, windowsOnMonitor);
    movedWindowData.rect_ = movedWindowData.previousRect_;
    if(overlappedWindowData.hwnd_ == nullptr) {
      movedWindowData.moveWindowToRect(movedWindowData.rect_, gapSize);
      return;
    }

    overlappedWindowData.rect_ = overlappedWindowData.getOriginalRect();
    swapWindows(movedWindowData, overlappedWindowData);

    return;
}

void swapWindows(WindowData& window1, WindowData& window2)
{
    if(window1.hwnd_ == window2.hwnd_)
        return;
    if(window1.hwnd_ == nullptr || window2.hwnd_ == nullptr)
        return;

    RECT window1Rect = window1.rect_;
    RECT window2Rect = window2.rect_;
    Container* window1Parent = window1.parent_;
    Container* window2Parent = window2.parent_;

    window1.moveWindowToRect(window2Rect, gapSize);
    window2.moveWindowToRect(window1Rect, gapSize);

    window1.parent_ = window2Parent;
    window2.parent_ = window1Parent;
    return;
}

WindowData& getOverlappedWindowData(WindowData& movedWindowData, std::vector<HWND> windowsOnMonitor)
{
    size_t biggestOverlap = 0;
    HWND biggestOverlapWindow = nullptr;
    for(auto window : windowsOnMonitor)
    {
        if(window == movedWindowData.hwnd_)
            continue;

        RECT movedRect = movedWindowData.rect_;
        RECT windowRect = windowMap[window].rect_;
        RECT intersection;
        if(IntersectRect(&intersection, &movedRect, &windowRect))
        {
            size_t overlap = (intersection.right - intersection.left) * (intersection.bottom - intersection.top);
            if(overlap > biggestOverlap)
            {
                biggestOverlap = overlap;
                biggestOverlapWindow = window;
            }
        }
    }

    if(biggestOverlapWindow == nullptr)
        return *new WindowData();

    return windowMap[biggestOverlapWindow];
}

void updateWindows(bool windowCountChanged)
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
    else
        onWindowMoved(currentMonitor, windowsOnMonitor);

    return;
}

void resetWindows()
{
    std::cout << "--- Resetting windows --- " << std::endl;

    if(root)
        root->printStructure();

    std::map<HWND, WindowData> windowMapCopy(windowMap);
    for(auto window : windowMapCopy)
    {
        if(!doesWindowExist(window.first)) // then window.first got closed
        {
            WindowData closedWindow = window.second;
            Container* parent = closedWindow.parent_;
            WindowData* originalClosedWindow = &windowMap.find(closedWindow.hwnd_)->second;
            windowMap.erase(window.first);
            if(parent == nullptr)
                return;

            parent->removeLeaf(originalClosedWindow);
            Desktop* otherLeaf = parent->m_leafs_.front();

            // Case 1: The parent had only windows
            if(otherLeaf->type_ == DesktopType::WINDOW)
            {
                std::cout << "--- Case 1 ---" << std::endl;
                WindowData* leafData = dynamic_cast<WindowData*>(otherLeaf);
                leafData->moveWindowToRect(parent->rect_, gapSize);
                std::cout << "--- Case 1 ---" << std::endl;
            }
            // Case 2: The parent had a leaf and a container
            else if(otherLeaf->type_ == DesktopType::CONTAINER)
            {
                std::cout << "--- Case 2 ---" << std::endl;
                RECT rectToFill = parent->rect_;
                Container* otherContainer = dynamic_cast<Container*>(otherLeaf);

                if(root == parent)
                    root = otherContainer;

                otherContainer->sizeUp(rectToFill);
                otherContainer->updateWindowPositions();
                otherContainer->toggleFormatDirection();
                std::cout << "--- Case 2 ---" << std::endl;
            }
            else
            {
                std::cout << "Case 3: Something went wrong" << std::endl;
                std::cout << "We have " << parent->getWindowCount() << " windows in the parent" << std::endl;
                std::cout << "And leaf[0] is of type: " << (int)otherLeaf->type_ << std::endl;
                exit(1);
            }

            Container* grandParent = parent->parent_;

            auto leafsCopy = parent->m_leafs_;
            for(auto leaf : leafsCopy)
            {
                Desktop* leafData = dynamic_cast<Desktop*>(leaf);

                if(leafData->type_ == DesktopType::WINDOW)
                    if(dynamic_cast<WindowData*>(leafData)->id_ == closedWindow.id_)
                        continue;

                if(grandParent == nullptr)
                {
                    std::cout << "Grandparent is nullptr" << std::endl;
                    leafData->parent_ = nullptr;
                    root = nullptr;
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

    if(root)
        root->printStructure();

    std::cout << "--- Done resetting windows --- " << std::endl;
    return;
}
