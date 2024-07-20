#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include <windows.h>

enum class FormatDirection
{
    VERTICAL,
    HORIZONTAL
};

enum class DesktopType
{
    WINDOW,
    CONTAINER
};

class Desktop {
public:
  void virtual printStructure() = 0;
  DesktopType type_;

};

class Container : public Desktop
{
public:
    static std::string lastId;
    DesktopType type_ = DesktopType::CONTAINER;
    std::string id_;
    Container();
    ~Container();
    RECT rect_;
    std::vector<Desktop*> m_leafs_; 
    Container* m_parent_ = nullptr;

    void addLeaf(Desktop* leaf);
    void removeLeaf(Desktop* leaf);
    int getWindowCount();
    void printStructure();
};

class WindowData : public Desktop
{
public:
    static size_t lastId;
    DesktopType type_ = DesktopType::WINDOW;
    size_t id_;
    BOOL sizePinned_ = false;
    BOOL positionPinned_ = false;
    int zIndex_ = 0;
    HWND hwnd_;
    std::string title_;
    RECT rect_;
    RECT previousRect_;
    RECT rectBeforeSplit_;
    FormatDirection formatDirection_ = FormatDirection::VERTICAL;
    struct WindowData* nextWindow_ = nullptr;
    struct WindowData* previousWindow_ = nullptr;
    Container* parent_ = nullptr;
    void printStructure();
};

extern int gap;
extern int borderGap;

extern int taskBarHeight;
extern std::vector<HWND> windows;
extern std::map<HWND, WindowData> windowMap;
extern std::vector<std::string> blacklist;
extern int amountOfWindows;
extern int prevAmountOfWindows;

bool operator==(const RECT& lhs, const RECT& rhs);

BOOL CALLBACK saveWindow(HWND hwnd, LPARAM substring);
int GetTaskBarHeight();
bool doesWindowExist(HWND hwnd);
void toggleFormatDirection(HWND hwnd);
bool DidWindowPositionChange();
