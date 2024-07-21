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
    WINDOW = 0,
    CONTAINER = 1,
    DESKTOP = 2
};


class Container;
class WindowData;

class Desktop
{
public:
    void virtual printStructure() = 0;
    DesktopType type_;
    RECT rect_;
    FormatDirection formatDirection_ = FormatDirection::VERTICAL;
    void toggleFormatDirection();
    Container* parent_ = nullptr;
    
};


class Container : public Desktop
{
public:
    static std::string lastId;
    std::string id_;
    Container();
    ~Container();
    std::vector<Desktop*> m_leafs_;

    void addLeaf(Desktop* leaf);
    void removeLeaf(Desktop* leaf);
    std::vector<std::pair<WindowData*, size_t>> getAllWindows(int depth = 1);
    void sizeUp(RECT rect);
    int getWindowCount();
    void printStructure();
    void updateWindowPositions();
};

class WindowData : public Desktop
{
public:
    static size_t lastId;
    size_t id_;
    BOOL sizePinned_ = false;
    BOOL positionPinned_ = false;
    int zIndex_ = 0;
    HWND hwnd_;
    std::string title_;
    RECT previousRect_;
    struct WindowData* nextWindow_ = nullptr;
    struct WindowData* previousWindow_ = nullptr;
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
bool DidWindowPositionChange();
void MoveWindowToRect(HWND hwnd, RECT rect);
