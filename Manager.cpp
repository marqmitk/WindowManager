#include "Manager.hpp"
#include "Containers.hpp"
#include "Utils.hpp"
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
    WindowData* nWindowData = windowMap[windowsOnMonitor[0]];

    if(windowsOnMonitor.size() == 1)
    {
        top = currentMonitor.rcMonitor.top + 2 * gapSize;
        left = currentMonitor.rcMonitor.left + gapSize;
        right = currentMonitor.rcMonitor.right - gapSize;
        bottom = currentMonitor.rcMonitor.bottom - (gapSize / 2) - taskBarHeight;
        RECT rect = {left, top, right - left, bottom - top};
        nWindowData->moveWindowToRect(rect);
        return;
    }

    WindowData* aWindowData = windowMap[windowsOnMonitor[1]];

    aWindowData->rect_ = aWindowData->getOriginalRect();
    nWindowData->rect_ = nWindowData->getOriginalRect();
    top = aWindowData->rect_.top;
    left = aWindowData->rect_.left;
    right = aWindowData->rect_.right;
    bottom = aWindowData->rect_.bottom;

    int resolutionX = right - left;
    int resolutionY = bottom - top;

    Container* newParent = new Container();
    if(aWindowData->parent_)
    {
        aWindowData->parent_->removeLeaf(aWindowData);
        aWindowData->parent_->addLeaf(newParent);
        newParent->parent_ = aWindowData->parent_;
        aWindowData->parent_ = newParent;
    }
    else
    {
        root = newParent;
    }

    newParent->rect_ = aWindowData->rect_;
    newParent->addLeaf(nWindowData);
    newParent->addLeaf(aWindowData);
    aWindowData->parent_ = newParent;
    nWindowData->parent_ = newParent;

    RECT aRect;
    RECT nRect;

    if(aWindowData->formatDirection_ == FormatDirection::VERTICAL)
    {
        aRect = {left, top, left + resolutionX / 2, top + resolutionY};
        nRect = {left + resolutionX / 2, top, right, bottom};
    }
    else if(aWindowData->formatDirection_ == FormatDirection::HORIZONTAL)
    {
        aRect = {left, top, right, top + resolutionY / 2};
        nRect = {left, top + resolutionY / 2, right, bottom};
    }

    aWindowData->moveWindowToRect(aRect);
    nWindowData->moveWindowToRect(nRect);

    aWindowData->toggleFormatDirection();
    nWindowData->formatDirection_ = aWindowData->formatDirection_;
    newParent->formatDirection_ = aWindowData->formatDirection_;

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
    WindowData* movedWindowData = windowMap[lastWindowGettingMoved];
    WindowData* overlappedWindowData = getOverlappedWindowData(movedWindowData, windowsOnMonitor);

    if(overlappedWindowData == nullptr)
    {
        movedWindowData->moveWindowToRect(movedWindowData->originalRect_);
        return;
    }

    movedWindowData->rect_ = movedWindowData->previousRect_;
    if(overlappedWindowData->hwnd_ == nullptr)
    {
        movedWindowData->moveWindowToRect(movedWindowData->rect_);
        return;
    }

    overlappedWindowData->rect_ = overlappedWindowData->getOriginalRect();
    swapWindows(movedWindowData, overlappedWindowData);

    return;
}

void swapWindows(WindowData* window1, WindowData* window2)
{
    if(window1->hwnd_ == window2->hwnd_)
        return;
    if(window1->hwnd_ == nullptr || window2->hwnd_ == nullptr)
        return;

    RECT window1Rect = window1->rect_;
    RECT window2Rect = window2->rect_;
    RECT window1PreviousRect = window1->previousRect_;
    RECT window2PreviousRect = window2->previousRect_;

    Container* window1Parent = window1->parent_;
    Container* window2Parent = window2->parent_;

    window1->moveWindowToRect(window2Rect);
    window2->moveWindowToRect(window1Rect);
    window1->previousRect_ = window2PreviousRect;
    window2->previousRect_ = window1PreviousRect;

    if(window1Parent != nullptr)
    {
        window1Parent->removeLeaf(window1);
        window1Parent->addLeaf(window2);
    }
    if(window2Parent != nullptr)
    {
        window2Parent->removeLeaf(window2);
        window2Parent->addLeaf(window1);
    }

    window1->parent_ = window2Parent;
    window2->parent_ = window1Parent;
    window1->formatDirection_ = window2->formatDirection_;
    window2->formatDirection_ = window1->formatDirection_;
    window1->id_ = window2->id_;
    window2->id_ = window1->id_;
    return;
}

WindowData* getOverlappedWindowData(WindowData* movedWindowData, std::vector<HWND> windowsOnMonitor)
{
    size_t biggestOverlap = 0;
    HWND biggestOverlapWindow = nullptr;
    for(auto window : windowsOnMonitor)
    {
        if(window == movedWindowData->hwnd_)
            continue;

        RECT movedRect = movedWindowData->rect_;
        RECT windowRect = windowMap[window]->rect_;
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
        return nullptr;

    return windowMap[biggestOverlapWindow];
}

Direction isGettingResized(HWND windowGettingMoved)
// -1 = not getting resized, 0 = topNeg, 1 = topPos, 2 = leftNeg, 3 = leftPos, 4 = rightNeg, 5 = rightPos, 6 = bottomNeg, 7 = bottomPos
{
    if(windowGettingMoved == nullptr)
        return -1;

    RECT currentRect;
    GetWindowRect(windowGettingMoved, &currentRect);
    windowMap[windowGettingMoved]->rect_ = currentRect;
    bool topNeg = currentRect.top < windowMap[windowGettingMoved]->previousRect_.top;
    bool topPos = currentRect.top > windowMap[windowGettingMoved]->previousRect_.top;
    bool leftNeg = currentRect.left < windowMap[windowGettingMoved]->previousRect_.left;
    bool leftPos = currentRect.left > windowMap[windowGettingMoved]->previousRect_.left;
    bool rightNeg = currentRect.right < windowMap[windowGettingMoved]->previousRect_.right;
    bool rightPos = currentRect.right > windowMap[windowGettingMoved]->previousRect_.right;
    bool bottomNeg = currentRect.bottom < windowMap[windowGettingMoved]->previousRect_.bottom;
    bool bottomPos = currentRect.bottom > windowMap[windowGettingMoved]->previousRect_.bottom;

    if(!topNeg && !topPos && !leftNeg && !leftPos && !rightNeg && !rightPos && !bottomNeg && !bottomPos)
        return -1;

    // return 0 if we are moving
    // we are moving if the opposite side is also moving
    if(topNeg && bottomPos)
        return -1;
    if(topPos && bottomNeg)
        return -1;
    if(leftNeg && rightPos)
        return -1;
    if(leftPos && rightNeg)
        return -1;
    if(topNeg && bottomNeg)
        return -1;
    if(topPos && bottomPos)
        return -1;
    if(leftNeg && rightNeg)
        return -1;
    if(leftPos && rightPos)
        return -1;

    std::cout << "Getting resized: " << topNeg << topPos << leftNeg << leftPos << rightNeg << rightPos << bottomNeg << bottomPos << std::endl;

    if(topNeg)
        return TOP_NEGATIVE;
    if(topPos)
        return TOP;
    if(leftNeg)
        return LEFT_NEGATIVE;
    if(leftPos)
        return LEFT;
    if(rightNeg)
        return RIGHT_NEGATIVE;
    if(rightPos)
        return RIGHT;
    if(bottomNeg)
        return BOTTOM_NEGATIVE;
    if(bottomPos)
        return BOTTOM;

    return -1;
}

void handleWindowMovement()
{
    HWND windowGettingMoved = getWindowGettingMoved();

    if(windowGettingMoved != lastWindowGettingMoved && lastWindowGettingMoved != nullptr) // We moved a window
    {
        if(isGettingResized(windowGettingMoved) == -1)
            updateWindows();
        else
            std::cout << "Wrong alarm" << std::endl;

        lastWindowGettingMoved = windowGettingMoved;
    }
    else if(windowGettingMoved != nullptr && lastWindowGettingMoved == nullptr) // First time we move a window
    {
        lastWindowGettingMoved = windowGettingMoved;
        windowMap[lastWindowGettingMoved]->previousRect_ = windowMap[lastWindowGettingMoved]->getOriginalRect();
    }
}

void updateNeighbours()
{
    for(auto window : windows)
    {
        WindowData* windowData = windowMap[window];
        windowData->neighbours_->clearNeighbours();
        windowData->fillNeighbours(gapSize);
    }

    for(auto container : containers)
    {
        container->neighbours_->clearNeighbours();
        container->fillNeighbours(gapSize);
    }
}

long calculateOffset(RECT rect1, RECT rect2, Direction direction)
{
    switch(direction)
    {
    case LEFT:
        return rect2.left - rect1.left;
    case TOP:
        return rect2.top - rect1.top;
    case RIGHT:
        return rect2.right - rect1.right;
    case BOTTOM:
        return rect2.bottom - rect1.bottom;
    }
    return 0;
}

void handleWindowResize()
{
    HWND windowGettingMoved = getWindowGettingMoved();
    if(windowGettingMoved == nullptr) // handle resetting invalid resizes
    {
        if(lastWindowGettingResized != nullptr)
        {
            WindowData* windowGettingResizedData = windowMap[lastWindowGettingResized];
            std::vector<WindowData*> adjecentWindows = windowGettingResizedData->neighbours_->getNeighbours(windowGettingResizedData->lastResizeDirection_);
            lastWindowGettingResized = nullptr;
            if(adjecentWindows.size() == 0)
            {
                windowGettingResizedData->moveWindowToRect(windowGettingResizedData->originalRect_);
                windowGettingResizedData->previousRect_ = windowGettingResizedData->rect_;
                return;
            }
            if(lastWindowGettingMoved != nullptr)
                windowMap[lastWindowGettingMoved]->previousRect_ = windowMap[lastWindowGettingMoved]->rect_;
        }
        return;
    }

    WindowData* windowGettingMovedData = windowMap[windowGettingMoved];

    Direction resizeDirection = isGettingResized(windowGettingMoved); // -1 = not getting resized
    if(resizeDirection == -1)
        return;

    lastWindowGettingResized = windowGettingMoved;
    resizeDirection = NORMALIZE_DIRECTION(resizeDirection);
    windowGettingMovedData->lastResizeDirection_ = resizeDirection;

    long offset = calculateOffset(windowGettingMovedData->rect_, windowGettingMovedData->previousRect_, resizeDirection);
    RECT rectAfterMove = windowGettingMovedData->rect_;
    std::cout << "Offset: " << offset << " Direction: " << resizeDirection << std::endl;

    Neighbours* neighbours = windowGettingMovedData->neighbours_;

    std::vector<WindowData*> adjecentWindows = neighbours->getNeighbours(resizeDirection);

    for(WindowData* neighbourWindow : adjecentWindows)
        neighbourWindow->pushResizeWindow(resizeDirection, offset);

    std::vector<Container*> adjecentContainers = neighbours->getContainerNeighbours(resizeDirection);
    for(auto container : adjecentContainers)
        container->pushResize(resizeDirection, offset);

    // get all siblings
    std::vector<WindowData*> siblings = windowGettingMovedData->getAllWindowSiblings();

    bool resizedSiblings = false;
    for(auto sibling : siblings)
    {
        bool found = false;
        for(WindowData* adWindow : adjecentWindows)
        {
            std::cout << "Sibling: " << sibling->id_ << " Adjecent window: " << adWindow->id_ << std::endl;
            if(sibling == adWindow)
            {
                found = true;
                break;
            }
        }

        if(found)
            continue;

        std::vector<WindowData*> nonMatchingNeighbours = {};
        std::vector<Container*> nonMatchingContainers = {};
        std::vector<Container*> matchingContainers = {};
        std::vector<WindowData*> siblingAdjecentWindows = sibling->neighbours_->getNeighbours(resizeDirection);
        std::vector<Container*> siblingAdjecentContainers = sibling->neighbours_->getContainerNeighbours(resizeDirection);

        // if the siling is to the 90 degree direction of the resize direction, we should not use it to resize others
        bool is90Degree = false;
        if(windowGettingMovedData->isWindowInDirection(sibling, NINTY_DEGREES_CW(resizeDirection)))
            is90Degree = true;
        else if(windowGettingMovedData->isWindowInDirection(sibling, NINTY_DEGREES_CCW(resizeDirection)))
            is90Degree = true;

        if(is90Degree)
        {
            std::cout << "90 degree" << std::endl;
            for(auto neighbour : siblingAdjecentWindows)
                for(auto neighbourOfWindowGettingMoved : adjecentWindows)
                    if(neighbour->hwnd_ != neighbourOfWindowGettingMoved->hwnd_)
                        nonMatchingNeighbours.push_back(neighbour);

            for(auto container : siblingAdjecentContainers)
                for(auto containerOfWindowGettingMoved : adjecentContainers)
                    if(container->id_ != containerOfWindowGettingMoved->id_)
                        nonMatchingContainers.push_back(container);

            for(auto notMatching : nonMatchingNeighbours)
                notMatching->pushResizeWindow(resizeDirection, offset);

            for(auto notMatching : nonMatchingContainers)
                notMatching->pushResize(offset, resizeDirection);

            sibling->resizeWindow(resizeDirection, offset);
        }
        windowMap[sibling->hwnd_]->previousRect_ = windowMap[sibling->hwnd_]->rect_;
        resizedSiblings = true;
    }

    std::vector<Container*> parentContainers = windowGettingMovedData->getAllParentContainers();
    std::cout << "Parent containers: " << parentContainers.size() << std::endl;
    if(resizedSiblings)
    {
        for(auto container : parentContainers)
        {
            if(container == root)
                continue;

            container->resize(resizeDirection, offset);
        }
    }

    windowMap[windowGettingMoved]->previousRect_ = rectAfterMove;
    windowMap[windowGettingMoved]->originalRect_ = rectAfterMove;
    return;
}

void updateWindows(bool windowCountChanged)
{
    if(windowCountChanged)
        std::cout << "Window count changed" << std::endl;
    else
        std::cout << "Window moved" << std::endl;

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

    updateNeighbours();
    return;
}

void resetWindows()
{
    std::cout << "--- Resetting windows --- " << std::endl;

    if(root)
        root->printStructure();

    std::map<HWND, WindowData*> windowMapCopy(windowMap);
    for(auto window : windowMapCopy)
    {
        if(!doesWindowExist(window.first)) // then window.first got closed
        {
            WindowData* closedWindow = window.second;
            Container* parent = closedWindow->parent_;
            WindowData* originalClosedWindow = windowMap[closedWindow->hwnd_];
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
                leafData->moveWindowToRect(parent->rect_);
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
                    if(dynamic_cast<WindowData*>(leafData)->id_ == closedWindow->id_)
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
    std::cout << "--- Done resetting windows 1 --- " << std::endl;
    if(root)
        root->printStructure();

    std::cout << "--- Done resetting windows --- " << std::endl;
    return;
}
