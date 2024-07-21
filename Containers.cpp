#include "Containers.hpp"

std::string Container::lastId = "a";
size_t WindowData::lastId = 0;

void Desktop::toggleFormatDirection()
{
    if(this->formatDirection_ == FormatDirection::VERTICAL)
        this->formatDirection_ = FormatDirection::HORIZONTAL;
    else
        this->formatDirection_ = FormatDirection::VERTICAL;
}

Container::Container()
{
    this->id_ = Container::lastId[0];
    Container::lastId[0]++;
    this->type_ = DesktopType::CONTAINER;
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
            std::cout << "Starting recursion" << std::endl;
            Container* container = dynamic_cast<Container*>(leaf);
            std::vector<std::pair<WindowData*, size_t>> containerWindows = container->getAllWindows(depth + 1);
            windowList.insert(windowList.end(), containerWindows.begin(), containerWindows.end());
            std::cout << "Ending recursion" << std::endl;
        }
    }
    return windowList;
}

void Container::sizeUp(RECT rect)
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
            window->rect_ = rect;
        }
        else if(leaf->type_ == DesktopType::CONTAINER)
        {
            std::cout << "Sizing container to " << rect.left << " " << rect.top << " " << rect.right << " " << rect.bottom << std::endl;
            Container* container = dynamic_cast<Container*>(leaf);
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
    {
        std::cout << "<" << this->id_ << " parent='" << parent->id_ << "'>" << std::endl;
    }
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

void WindowData::printStructure(int depth)
{
    for(int i = 0; i < depth; i++)
        std::cout << "  ";
    if(this->parent_ == nullptr)
        std::cout << "<window title='" << this->title_ << "' id='" << this->id_ << "' parent='None'/>" << std::endl;
    else
        std::cout << "<window title='" << this->title_ << "' id='" << this->id_ << "' parent='" << this->parent_->id_ << "'/>" << std::endl;
}

void WindowData::moveWindowToRect(RECT rect, int gap)
{
    RECT rectToMove = rect;
    rectToMove.left += gap;
    rectToMove.top += gap;
    rectToMove.right -= gap;
    rectToMove.bottom -= gap;

    this->rect_ = rect;
    this->originalRect_ = this->rect_;

    MoveWindow(this->hwnd_, rectToMove.left, rectToMove.top, rectToMove.right - rectToMove.left, rectToMove.bottom - rectToMove.top, TRUE);
}

RECT WindowData::getOriginalRect()
{
    if(this->originalRect_.left == 0 && this->originalRect_.top == 0 && this->originalRect_.right == 0 && this->originalRect_.bottom == 0)
        return this->rect_;
    return this->originalRect_;
}
