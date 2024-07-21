#include "utils.hpp"
#include <iostream>
#include <ostream>

int amountOfWindows = 0;
int prevAmountOfWindows = 0;

int gap = 3;
int borderGap = 3;

std::vector<HWND> windows;
std::map<HWND, WindowData> windowMap;
std::vector<std::string> blacklist = {
    "Windows-Eingabeerfahrung",
    "NVIDIA GeForce Overlay",
    "Program Manager",
    "Programmumschaltung",
    "PopupHost",
    "Andockhilfe",
    "WinUI Desktop",
    "PowerToys.MeasureToolOverlay",
    "Überlauffenster der Taskleiste.",
    "PowerToys Find My Mouse",
    "Ausführen",
};

std::string Container::lastId = "a";
size_t WindowData::lastId = 0;

void MoveWindowToRect(HWND hwnd, RECT rect)
{
    MoveWindow(hwnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
}

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

Container::~Container()
{
    for(auto leaf : m_leafs_)
    {
        if(leaf->type_ == DesktopType::WINDOW)
        {
            WindowData* window = dynamic_cast<WindowData*>(leaf);
            std::cout << "Container '" << this->id_ << "' is  getting deleted but still has window '" << window->title_ << "'" << std::endl;
        }
        else
        {
            Container* container = dynamic_cast<Container*>(leaf);
            std::cout << "Container '" << this->id_ << "' is  getting deleted but still has container '" << container->id_ << "'" << std::endl;
        }
    }
}

void Container::addLeaf(Desktop* leaf)
{
    m_leafs_.push_back(leaf);
}

void Container::removeLeaf(Desktop* leaf)
{
    m_leafs_.erase(std::remove(m_leafs_.begin(), m_leafs_.end(), leaf), m_leafs_.end());
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
            MoveWindowToRect(window->hwnd_, window->rect_);
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

void Container::printStructure()
{
    Container* parent = this->parent_;
    if(parent == nullptr)
        std::cout << "--- " << this->id_ << " --- with no parent" << std::endl;
    else
    {
        std::cout << "--- " << this->id_ << " --- with parent " << parent->id_ << std::endl;
    }
    int i = 0;
    for(auto leaf : m_leafs_)
    {
        std::cout << i++ << ": ";
        if(leaf == nullptr)
            std::cout << "nullptr" << std::endl;
        else
            leaf->printStructure();
    }
    std::cout << "--- " << this->id_ << " ---" << std::endl;
}

void WindowData::printStructure()
{
    std::cout << this->id_ << " " << this->title_ << std::endl;
}

int taskBarHeight;

bool operator==(const RECT& lhs, const RECT& rhs)
{ // Compares two RECT structs
    return lhs.left == rhs.left && lhs.right == rhs.right && lhs.top == rhs.top && lhs.bottom == rhs.bottom;
}

BOOL CALLBACK saveWindow(HWND hwnd, LPARAM substring)
{

    const DWORD titleSize = 1024;
    TCHAR windowTitle[titleSize];

    GetWindowText(hwnd, windowTitle, titleSize);
    int length = ::GetWindowTextLength(hwnd);

    std::string temp(&windowTitle[0]);
    std::string title(temp.begin(), temp.end());

    if(!IsWindowVisible(hwnd) || length == 0)
        return TRUE;

    for(auto blacklisted : blacklist)
        if(strncmp(title.c_str(), blacklisted.c_str(), blacklisted.size()) == 0)
            return TRUE;

    windowMap[hwnd].title_ = title;
    windowMap[hwnd].hwnd_ = hwnd;
    // get coordinates
    GetWindowRect(hwnd, &windowMap[hwnd].rect_);
    windowMap[hwnd].id_ = WindowData::lastId++;
    windowMap[hwnd].type_ = DesktopType::WINDOW;

    windows.push_back(hwnd);
    amountOfWindows++;

    return TRUE;
}

int GetTaskBarHeight()
{
    HWND taskBar = FindWindowEx(0, 0, "Shell_TrayWnd", 0);
    RECT taskBarRect;
    GetWindowRect(taskBar, &taskBarRect);
    return taskBarRect.bottom - taskBarRect.top;
}

bool doesWindowExist(HWND hwnd)
{
    return std::find(windows.begin(), windows.end(), hwnd) != windows.end();
}

bool DidWindowPositionChange()
{
    for(auto windowData : windowMap)
        if(!(windowData.second.rect_ == windowData.second.previousRect_))
            return true;

    return false;
}
