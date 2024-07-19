#include "Manager.h"
#include <algorithm>
#include <windef.h>

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
	int top;
	int left;
	int right;
	int bottom;
	WindowData newWindowData = windowMap[windowsOnMonitor[0]];

	if(windowsOnMonitor.size() == 1)
	{
		top = currentMonitor.rcMonitor.top + borderGap;
		left = currentMonitor.rcMonitor.left + borderGap;
		right = currentMonitor.rcMonitor.right - borderGap;
		bottom = currentMonitor.rcMonitor.bottom - borderGap - taskBarHeight;
		MoveWindow(newWindowData.hwnd_, left, top, right - left, bottom - top, TRUE);
		return;
	}

	WindowData activeWindowData = windowMap[windowsOnMonitor[1]];
	top = activeWindowData.rect_.top;
	left = activeWindowData.rect_.left;
	right = activeWindowData.rect_.right;
	bottom = activeWindowData.rect_.bottom;

	int resolutionX = right - left;
	int resolutionY = bottom - top;

	WindowData* aWindowData = &windowMap[activeWindowData.hwnd_];
	WindowData* nWindowData = &windowMap[newWindowData.hwnd_];

	aWindowData->previousRect_ = activeWindowData.rect_;
	nWindowData->previousRect_ = activeWindowData.rect_;

	aWindowData->nextWindow_ = nWindowData;
	nWindowData->previousWindow_ = aWindowData;

	if(activeWindowData.formatDirection_ == FormatDirection::VERTICAL)
	{
		std::cout << "Vertical" << std::endl;
		MoveWindow(activeWindowData.hwnd_, left, top, resolutionX / 2, resolutionY, TRUE);
		MoveWindow(newWindowData.hwnd_, left + resolutionX / 2, top, resolutionX / 2, resolutionY, TRUE);
	}
	else if(activeWindowData.formatDirection_ == FormatDirection::HORIZONTAL)
	{
		std::cout << "Horizontal" << std::endl;
		MoveWindow(activeWindowData.hwnd_, left, top, resolutionX, resolutionY / 2, TRUE);
		MoveWindow(newWindowData.hwnd_, left, top + resolutionY / 2, resolutionX, resolutionY / 2, TRUE);
	}
	std::cout << "Active window format direction before: " << (int)aWindowData->formatDirection_ << std::endl;
	toggleFormatDirection(activeWindowData.hwnd_);
	std::cout << "Active window format direction: after" << (int)aWindowData->formatDirection_ << std::endl;
	nWindowData->formatDirection_ = aWindowData->formatDirection_;
	std::cout << "New window format direction: " << (int)nWindowData->formatDirection_ << std::endl;
}

void resetWindowPosition(HWND hwnd)
{
	WindowData windowData = windowMap[hwnd];
	MoveWindow(hwnd,
			   windowData.previousRect_.left,
			   windowData.previousRect_.top,
			   windowData.previousRect_.right - windowData.previousRect_.left,
			   windowData.previousRect_.bottom - windowData.previousRect_.top,
			   TRUE);
	return;
}

void onWindowCountChanged()
{
	// Get current monitor
	MONITORINFO currentMonitor;
	HWND window = windows.front();
	currentMonitor.cbSize = sizeof(MONITORINFO);

	GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST), &currentMonitor);

	// Get windows on monitor
	std::vector<HWND> windowsOnMonitor = {};
	if(getWindowsOnMonitor(currentMonitor, windowsOnMonitor) == -1) // Fails if there is a maximized window
		return;

	// Format windows
	if(prevAmountOfWindows < amountOfWindows)
		splitWindow(currentMonitor, windowsOnMonitor);
	else if(prevAmountOfWindows > amountOfWindows)
		resetWindows();

	return;
}

void letWindowsFillSpace(RECT space, std::vector<HWND> windows)
{
	// TODO
	return;
}

void resetWindows()
{
	std::map<HWND, WindowData> windowMapCopy(windowMap);
	for(auto window : windowMapCopy)
	{
		if(!doesWindowExist(window.first))
		{
			WindowData* previousWindowData = window.second.previousWindow_;
			WindowData* nextWindowData = window.second.nextWindow_;

			if(nextWindowData)
			{
				resetWindowPosition(nextWindowData->hwnd_);
				toggleFormatDirection(nextWindowData->hwnd_);
				nextWindowData->previousWindow_ = nullptr;

				if(previousWindowData)
					previousWindowData->nextWindow_ = nullptr;

				continue;
			}

			if(previousWindowData)
			{
				std::cout << "Prev Pointer to previous WindowData " << previousWindowData << std::endl;
				std::cout << "Prev Pointer to next WindowData " << nextWindowData << std::endl;
				resetWindowPosition(previousWindowData->hwnd_);
				toggleFormatDirection(previousWindowData->hwnd_);
				previousWindowData->nextWindow_ = nullptr;

				if(nextWindowData)
					nextWindowData->previousWindow_ = nullptr;
			}
			windowMap.erase(window.first);
			std::cout << "Finished handling resetting" << std::endl;
		}
	}
}
