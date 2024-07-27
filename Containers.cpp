#include "Containers.hpp"
#include "Manager.hpp"
#include "Utils.hpp"

std::string Container::lastId = "a";
size_t WindowData::lastId = 0;

void Desktop::toggleFormatDirection()
{
    if(this->formatDirection_ == FormatDirection::VERTICAL)
        this->formatDirection_ = FormatDirection::HORIZONTAL;
    else
        this->formatDirection_ = FormatDirection::VERTICAL;
}

long Desktop::getHeight()
{
    return this->rect_.bottom - this->rect_.top;
}

long Desktop::getWidth()
{
    return this->rect_.right - this->rect_.left;
}

bool Desktop::isSibling(Desktop* sibling)
{
    if(this->parent_ == nullptr)
        return false;
    if(this->parent_ == sibling->parent_)
        return true;
    return false;
}

bool Desktop::hasDirectSiblings(Direction direction)
{
    direction = NORMALIZE_DIRECTION(direction);
    if(this->parent_ == nullptr)
        return false;

    switch(direction)
    {
    case LEFT:
        for(auto neighbour : this->parent_->neighbours_->left_)
        {
            Desktop* currentParent = neighbour->parent_;
            while(currentParent != nullptr)
            {
                if(currentParent == this->parent_)
                    return true;
                currentParent = currentParent->parent_;
            }
        }
    case TOP:
        for(auto neighbour : this->parent_->neighbours_->top_)
            if(neighbour->isSibling(this))
                return true;
    case RIGHT:
        for(auto neighbour : this->parent_->neighbours_->right_)
            if(neighbour->isSibling(this))
                return true;
    case BOTTOM:
        for(auto neighbour : this->parent_->neighbours_->bottom_)
            if(neighbour->isSibling(this))
                return true;
    }
    return false;
}

bool Desktop::hasDirectSibling(Desktop* potentialSibling, Direction direction)
{
    direction = NORMALIZE_DIRECTION(direction);
    if(this->parent_ == nullptr)
        return false;

    switch(direction)
    {
    case LEFT:
        if(this->neighbours_->left_.empty())
            break;
        for(auto sibling : this->neighbours_->left_)
            if(sibling == potentialSibling)
                return true;
        break;
    case TOP:
        if(this->neighbours_->top_.empty())
            break;
        for(auto sibling : this->neighbours_->top_)
            if(sibling == potentialSibling)
                return true;
        break;
    case RIGHT:
        if(this->neighbours_->right_.empty())
            break;
        for(auto sibling : this->neighbours_->right_)
            if(sibling == potentialSibling)
                return true;
        break;
    case BOTTOM:
        if(this->neighbours_->bottom_.empty())
            break;
        for(auto sibling : this->neighbours_->bottom_)
            if(sibling == potentialSibling)
                return true;
        break;
    }
    return false;
}

Desktop* Desktop::getSibling()
{
    if(this->parent_ == nullptr)
        return nullptr;
    for(auto leaf : this->parent_->m_leafs_)
    {
        if(leaf == this)
            continue;
        return leaf;
    }
    return nullptr; // that happens when the container only has one child which should never happen
}

std::vector<WindowData*> Desktop::getAllWindowSiblings()
{
    std::vector<WindowData*> siblings = {};
    if(this->parent_ == nullptr)
        return siblings;
    auto windowPairs = this->parent_->getAllWindows();

    for(auto pair : windowPairs)
    {
        WindowData* window = pair.first;
        if(window != this)
            siblings.push_back(window);
    }
    return siblings;
}

Container::Container()
{
    this->id_ = Container::lastId[0];
    Container::lastId[0]++;
    this->type_ = DesktopType::CONTAINER;
    containers.push_back(this);
    this->neighbours_ = new Neighbours();
}

Container::~Container()
{
    std::cout << "Deleting container " << this->id_ << std::endl;
    containers.erase(std::remove(containers.begin(), containers.end(), this), containers.end());
}

void Container::addLeaf(Desktop* leaf)
{
    m_leafs_.push_back(leaf);
}

void Container::removeLeaf(Desktop* leaf)
{
    std::vector<Desktop*> newLeafs;
    for(auto l : m_leafs_)
    {
        if(l != leaf)
            newLeafs.push_back(l);
    }
    m_leafs_ = newLeafs;
}

bool Container::hasChild(Desktop* child)
{
    for(auto leaf : m_leafs_)
    {
        if(leaf == child)
        {
            return true;
        }
        else if(leaf->type_ == DesktopType::CONTAINER)
        {
            Container* container = dynamic_cast<Container*>(leaf);
            if(container->hasChild(child))
                return true;
        }
    }
    return false;
}

std::vector<std::pair<WindowData*, size_t>> Container::getAllWindows(int depth)
{
    std::vector<std::pair<WindowData*, size_t>> windowList;
    for(auto leaf : m_leafs_)
    {
        if(leaf->type_ == DesktopType::WINDOW)
        {
            std::pair<WindowData*, size_t> windowPair = std::make_pair(dynamic_cast<WindowData*>(leaf), depth);
            windowList.push_back(windowPair);
        }
        else
        {
            Container* container = dynamic_cast<Container*>(leaf);
            std::vector<std::pair<WindowData*, size_t>> containerWindows = container->getAllWindows(depth + 1);
            windowList.insert(windowList.end(), containerWindows.begin(), containerWindows.end());
        }
    }
    return windowList;
}

void Container::sizeUp(RECT rect, HWND exludeWindow)
{
    this->rect_ = rect;
    long width = rect.right - rect.left;
    long height = rect.bottom - rect.top;
    long x = rect.left;
    long y = rect.top;
    long leafWidth = width;
    long leafHeight = height;
    int i = 0; // can be at maximum 1
    for(auto leaf : m_leafs_)
    {
        if(formatDirection_ == FormatDirection::VERTICAL)
        {
            rect = {x, y, x + width, y + height / 2};
            if(i != 0)
                rect = {x, y + height / 2, x + width, y + height};
        }
        else
        {
            rect = {x, y, x + width / 2, y + height};
            if(i != 0)
                rect = {x + width / 2, y, x + width, y + height};
        }

        if(leaf->type_ == DesktopType::WINDOW)
        {
            std::cout << "Sizing window to " << rect.left << " " << rect.top << " " << rect.right << " " << rect.bottom << std::endl;
            WindowData* window = dynamic_cast<WindowData*>(leaf);
            if(window->hwnd_ == exludeWindow)
                continue;
            window->rect_ = rect;
        }
        else if(leaf->type_ == DesktopType::CONTAINER)
        {
            std::cout << "Sizing container to " << rect.left << " " << rect.top << " " << rect.right << " " << rect.bottom << std::endl;
            Container* container = dynamic_cast<Container*>(leaf);
            auto containerWindows = container->getAllWindows();
            bool found = false;
            for(auto window : containerWindows)
            {
                if(window.first->hwnd_ == exludeWindow)
                {
                    found = true;
                    break;
                }
            }
            if(found)
                continue;
            container->sizeUp(rect);
        }
        else
        {
            std::cout << "Type is: " << (int)leaf->type_ << std::endl;
            exit(1);
        }
        i++;
    }
}

void Container::updateWindowPositions()
{
    for(auto leaf : m_leafs_)
    {
        if(leaf->type_ == DesktopType::WINDOW)
        {
            WindowData* window = dynamic_cast<WindowData*>(leaf);
            window->moveWindowToRect(window->rect_);
        }
        else if(leaf->type_ == DesktopType::CONTAINER)
        {
            Container* container = dynamic_cast<Container*>(leaf);
            container->updateWindowPositions();
        }
        else
        {
            std::cout << "Type is: " << (int)leaf->type_ << std::endl;
            exit(1);
        }
    }
}

int Container::getWindowCount()
{
    size_t count = 0;
    for(auto leaf : m_leafs_)
        if(leaf->type_ == DesktopType::WINDOW)
            count++;
        else
            std::cout << "Type is: " << (int)leaf->type_ << std::endl;
    return count;
}

void Container::printStructure(int depth)
{
    for(int i = 0; i < depth; i++)
        std::cout << "  ";
    Container* parent = this->parent_;
    if(parent == nullptr)
        std::cout << "<" << this->id_ << " parent='None'>" << std::endl;
    else
        std::cout << "<" << this->id_ << " parent='" << parent->id_ << "'>" << std::endl;
    int i = 0;
    for(auto leaf : m_leafs_)
    {
        if(leaf == nullptr)
            std::cout << "nullptr" << std::endl;
        else
            leaf->printStructure(depth + 1);
    }
    for(int i = 0; i < depth; i++)
        std::cout << "  ";
    std::cout << "</" << this->id_ << ">" << std::endl;
}

void Container::resize(Direction direction, long offset)
{
    switch(direction)
    {
    case TOP:
        this->rect_.top -= offset;
        break;
    case LEFT:
        this->rect_.left -= offset;
        break;
    case RIGHT:
        this->rect_.right -= offset;
        break;
    case BOTTOM:
        this->rect_.bottom -= offset;
        break;
    }
}

void Container::pushResize(Direction direction, long offset)
{
    switch(direction)
    {
    case TOP:
        this->rect_.bottom -= offset;
        break;
    case LEFT:
        this->rect_.right -= offset;
        break;
    case RIGHT:
        this->rect_.left -= offset;
        break;
    case BOTTOM:
        this->rect_.top -= offset;
        break;
    }
}

void WindowData::printStructure(int depth)
{
    for(int i = 0; i < depth; i++)
        std::cout << "  ";
    if(this->parent_ == nullptr)
        std::cout << "<window hwnd='" << this->hwnd_ << "' title='" << this->title_ << "' id='" << this->id_ << "' parent='None'/>" << std::endl;
    else
        std::cout << "<window hwnd='" << this->hwnd_ << "' title='" << this->title_ << "' id='" << this->id_ << "' parent='" << this->parent_->id_ << "'/>" << std::endl;
}

void Desktop::fillNeighbours(int gap)
{
    RECT raytraceRect;
    bool found = false;
    int searchAmount = 0;
    int step = 1;
    for(Direction direction = LEFT; direction <= BOTTOM; direction++)
    {
        raytraceRect = this->rect_;
        found = false;
        searchAmount = 0;
        while(!found && searchAmount < 300) // We only want to search for windows within the gapSize, currently using 300 for debugging and until the gaps are fixed
        {
            for(auto hwnd : windows)
            {
                if(type_ == DesktopType::WINDOW && hwnd == dynamic_cast<WindowData*>(this)->hwnd_)
                    continue;
                if(type_ == DesktopType::CONTAINER)
                    if(dynamic_cast<Container*>(this)->hasChild(windowMap[hwnd]))
                        continue;

                WindowData* windowData = windowMap[hwnd];
                RECT windowRect = windowData->rect_;
                RECT intersection;
                if(IntersectRect(&intersection, &raytraceRect, &windowRect))
                {
                    neighbours_->addNeighbour(windowData, direction);
                    found = true;
                }
            }
            for(auto container : containers)
            {
                if(type_ == DesktopType::CONTAINER && container == dynamic_cast<Container*>(this))
                    continue;
                if(type_ == DesktopType::WINDOW)
                {
                    std::vector<Container*> parentContainers = dynamic_cast<WindowData*>(this)->getAllParentContainers(0);
                    if(std::find(parentContainers.begin(), parentContainers.end(), container) != parentContainers.end())
                        continue;
                }

                if(root == container)
                    continue;

                RECT containerRect = container->rect_;
                RECT intersection;
                if(IntersectRect(&intersection, &raytraceRect, &containerRect))
                    neighbours_->addNeighbour(container, direction);
            }

            switch(direction)
            {
            case LEFT:
                raytraceRect.left -= step;
                break;
            case TOP:
                raytraceRect.top -= step;
                break;
            case RIGHT:
                raytraceRect.right += step;
                break;
            case BOTTOM:
                raytraceRect.bottom += step;
                break;
            }
            searchAmount++;
        }
    }
}

bool WindowData::isWindowInDirection(WindowData* two, Direction direction)
{
    std::cout << "Checking if window << " << two->id_ << " is in direction " << direction << " of window " << this->id_ << std::endl;
    RECT twoRect = two->rect_;
    RECT oneRect = this->rect_;
    if(direction == 3)
    {
        std::cout << "One rect: " << oneRect.left << " " << oneRect.top << " " << oneRect.right << " " << oneRect.bottom << std::endl;
        std::cout << "Two rect: " << twoRect.left << " " << twoRect.top << " " << twoRect.right << " " << twoRect.bottom << std::endl;
    }

    switch(direction)
    {
    case TOP:
        if(twoRect.bottom <= oneRect.top && twoRect.top < oneRect.top)
            return true;
        break;
    case BOTTOM:
        if(twoRect.top <= oneRect.bottom && twoRect.bottom > oneRect.bottom)
            return true;
        break;
    case LEFT:
        if(twoRect.right <= oneRect.left && twoRect.left < oneRect.left)
            return true;
        break;
    case RIGHT:
        if(twoRect.left <= oneRect.right && twoRect.right > oneRect.right)
            return true;
        break;
    }
    return false;
}

void WindowData::moveWindowToRect(RECT rect, int gap)
{
    rect.left += gap;
    rect.top += gap;
    rect.right -= gap;
    rect.bottom -= gap;

    this->rect_ = rect;
    this->originalRect_ = this->rect_;

    MoveWindow(this->hwnd_, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
}

RECT WindowData::getOriginalRect()
{
    if(this->originalRect_.left == 0 && this->originalRect_.top == 0 && this->originalRect_.right == 0 && this->originalRect_.bottom == 0)
        return this->rect_;
    return this->originalRect_;
}

void WindowData::resizeWindow(Direction direction, long offset)
{
    RECT newRect = this->rect_;
    switch(direction)
    {
    case LEFT:
        newRect.left = newRect.left - offset;
        break;
    case RIGHT:
        newRect.right = newRect.right - offset;
        break;
    case TOP:
        newRect.top = newRect.top - offset;
        break;
    case BOTTOM:
        newRect.bottom = newRect.bottom - offset;
        break;
    }
    this->moveWindowToRect(newRect);
}

void WindowData::pushResizeWindow(Direction direction, long offset)
{
    RECT newRect = this->rect_;

    switch(direction)
    {
    case LEFT:
        newRect.right -= offset;
        break;
    case RIGHT:
        newRect.left -= offset;
        break;
    case TOP:
        newRect.bottom -= offset;
        break;
    case BOTTOM:
        newRect.top -= offset;
        break;
    }

    this->moveWindowToRect(newRect);
}

std::vector<Container*> WindowData::getAllParentContainers(bool withRoot)
{
    std::vector<Container*> tempContainers;
    Desktop* parent = this->parent_;
    while(parent != nullptr)
    {
        if(parent->type_ == DesktopType::CONTAINER)
            if(withRoot || parent != root)
                tempContainers.push_back(dynamic_cast<Container*>(parent));

        parent = parent->parent_;
    }
    return tempContainers;
}

void WindowData::closeWindow()
{
    SendMessage(this->hwnd_, WM_CLOSE, 0, 0);
}

WindowData::~WindowData()
{
  std::cout << "Deleting window " << this->id_ << std::endl;
    windows.erase(std::remove(windows.begin(), windows.end(), this->hwnd_), windows.end());
    windowMap.erase(this->hwnd_);
    if(this->parent_ != nullptr)
        this->parent_->removeLeaf(this);
    for(auto pair : windowMap)
    {
        WindowData* window = pair.second;
        if(window->neighbours_->noNeighbours())
            continue;
        window->neighbours_->removeNeighbour(this);
    }
    delete this->neighbours_;
}

void Neighbours::addNeighbour(WindowData* window, Direction direction)
{
    switch(direction)
    {
    case LEFT:
        left_.push_back(window);
        break;
    case TOP:
        top_.push_back(window);
        break;
    case RIGHT:
        right_.push_back(window);
        break;
    case BOTTOM:
        bottom_.push_back(window);
        break;
    }
}

void Neighbours::addNeighbour(Container* container, Direction direction)
{
    switch(direction)
    {
    case LEFT:
        leftContainers_.push_back(container);
        break;
    case TOP:
        topContainers_.push_back(container);
        break;
    case RIGHT:
        rightContainers_.push_back(container);
        break;
    case BOTTOM:
        bottomContainers_.push_back(container);
        break;
    }
}

void Neighbours::clearNeighbours(Direction direction) // if direction is -1, clear all
{
    if(direction == -1)
    {
        left_.clear();
        top_.clear();
        right_.clear();
        bottom_.clear();
    }
    else
    {
        switch(direction)
        {
        case LEFT:
            left_.clear();
            break;
        case TOP:
            top_.clear();
            break;
        case RIGHT:
            right_.clear();
            break;
        case BOTTOM:
            bottom_.clear();
            break;
        }
    }
}

bool Neighbours::noNeighbours()
{
    return left_.empty() && top_.empty() && right_.empty() && bottom_.empty();
}

std::vector<WindowData*> Neighbours::getNeighbours(Direction direction)
{
    direction = NORMALIZE_DIRECTION(direction);
    switch(direction)
    {
    case LEFT:
        return left_;
    case TOP:
        return top_;
    case RIGHT:
        return right_;
    case BOTTOM:
        return bottom_;
    case -1:
        std::vector<WindowData*> allNeighbours;
        allNeighbours.insert(allNeighbours.end(), left_.begin(), left_.end());
        allNeighbours.insert(allNeighbours.end(), top_.begin(), top_.end());
        allNeighbours.insert(allNeighbours.end(), right_.begin(), right_.end());
        allNeighbours.insert(allNeighbours.end(), bottom_.begin(), bottom_.end());
        return allNeighbours;
    }
    return {};
}

std::vector<Container*> Neighbours::getContainerNeighbours(Direction direction)
{
    direction = NORMALIZE_DIRECTION(direction);
    switch(direction)
    {
    case LEFT:
        return leftContainers_;
    case TOP:
        return topContainers_;
    case RIGHT:
        return rightContainers_;
    case BOTTOM:
        return bottomContainers_;
    }
    return std::vector<Container*>();
}

void Neighbours::removeNeighbour(WindowData* window)
{
    for(auto it = left_.begin(); it != left_.end(); it++)
    {
        if(*it == window)
        {
            left_.erase(it);
            return;
        }
    }
    for(auto it = top_.begin(); it != top_.end(); it++)
    {
        if(*it == window)
        {
            top_.erase(it);
            return;
        }
    }
    for(auto it = right_.begin(); it != right_.end(); it++)
    {
        if(*it == window)
        {
            right_.erase(it);
            return;
        }
    }
    for(auto it = bottom_.begin(); it != bottom_.end(); it++)
    {
        if(*it == window)
        {
            bottom_.erase(it);
            return;
        }
    }
}
