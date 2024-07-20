#include "utils.h"
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

Container::Container()
{
    this->id_ = Container::lastId[0];
    Container::lastId[0]++;
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

int Container::getWindowCount()
{
    size_t count = 0;
    for(auto leaf : m_leafs_)
        if(leaf->type_ == DesktopType::WINDOW)
            count++;
    return count;
}

void Container::printStructure()
{
    Desktop* parent = this->m_parent_;
    if(parent == nullptr)
      std::cout << "--- " << this->id_ << " --- with no parent" << std::endl;
    else {
      if(parent->type_ == DesktopType::WINDOW)
        std::cout << "--- " << this->id_ << " --- with parent " << dynamic_cast<WindowData*>(parent)->id_ << std::endl;
      else
        std::cout << "--- " << this->id_ << " --- with parent " << dynamic_cast<Container*>(parent)->id_ << std::endl;
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

void toggleFormatDirection(HWND hwnd)
{
    if(windowMap[hwnd].formatDirection_ == FormatDirection::HORIZONTAL)
        windowMap[hwnd].formatDirection_ = FormatDirection::VERTICAL;
    else
        windowMap[hwnd].formatDirection_ = FormatDirection::HORIZONTAL;
    return;
}

bool DidWindowPositionChange()
{
    for(auto windowData : windowMap)
        if(!(windowData.second.rect_ == windowData.second.previousRect_))
            return true;

    return false;
}
