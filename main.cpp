#include "Manager.h"
#include <winuser.h>

BOOL running = true;

int main() {

  taskBarHeight = GetTaskBarHeight();
  std::cout << "Taskbar height: " << taskBarHeight << std::endl;

  EnumWindows(saveWindow, 0);
  amountOfWindows = windows.size();
  prevAmountOfWindows = amountOfWindows;
  std::cout << "Ready" << std::endl;

  while (running) {
    if (GetAsyncKeyState(VK_F2) & 0x8000)
      exit(0);

    windows.clear();
    amountOfWindows = 0;
    EnumWindows(saveWindow, 0);
    if (amountOfWindows != prevAmountOfWindows)
      onWindowCountChanged();

    prevAmountOfWindows = amountOfWindows;
  }

  return 0;
}
