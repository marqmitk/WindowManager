#include <string>
#include <algorithm>
#include <vector>
#include <windows.h>
#include <map>

enum class FormatDirection {
  VERTICAL,
  HORIZONTAL
};

struct WindowData {
  BOOL sizePinned_ = false;
  BOOL positionPinned_ = false;
  int zIndex_ = 0;
  HWND hwnd_;
  std::string title_;
  RECT rect_;
  RECT previousRect_;
  FormatDirection formatDirection_ = FormatDirection::VERTICAL;
  struct WindowData *nextWindow_ = nullptr;
  struct WindowData *previousWindow_ = nullptr;
};


extern int gap;
extern int borderGap;

extern int taskBarHeight;
extern std::vector<HWND> windows;
extern std::map<HWND, WindowData> windowMap;
extern std::vector<std::string> blacklist;
extern int amountOfWindows;
extern int prevAmountOfWindows;

bool operator==(const RECT &lhs, const RECT &rhs);

BOOL CALLBACK saveWindow(HWND hwnd, LPARAM substring);
int GetTaskBarHeight();
bool doesWindowExist(HWND hwnd);
void toggleFormatDirection(HWND hwnd);
