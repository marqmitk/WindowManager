#pragma once
#include "Config.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <windows.h>

// enum class Direction
// {
//   LEFT,
//   TOP,
//   RIGHT,
//   BOTTOM
// };

typedef int Direction; // not sure if thats smarter than a enum
#define LEFT 0
#define TOP 1
#define RIGHT 2
#define BOTTOM 3
#define LEFT_NEGATIVE 4
#define TOP_NEGATIVE 5
#define RIGHT_NEGATIVE 6
#define BOTTOM_NEGATIVE 7
#define NORMALIZE_DIRECTION(direction) (direction % 4)
#define OPPOSITE_DIRECTION(direction) ((direction + 2) % 4)

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
class Neighbours;

class Desktop
{
public:
    void virtual printStructure(int depth = 0) = 0;
    DesktopType type_;
    RECT rect_;
    long getHeight();
    long getWidth();
    FormatDirection formatDirection_ = FormatDirection::VERTICAL;
    void toggleFormatDirection();
    Container* parent_ = nullptr;
    Neighbours* neighbours_;
    void fillNeighbours(int gap);
    Desktop* getSibling();
    std::vector<WindowData*> getAllWindowSiblings();
    bool isSibling(Desktop* sibling);
    bool hasDirectSibling(Desktop* potentialSibling, Direction direction);
    bool hasDirectSiblings(Direction direction);
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
    void sizeUp(RECT rect, HWND exludeWindow = nullptr);
    int getWindowCount();
    void printStructure(int depth = 0);
    void updateWindowPositions();
    bool hasChild(Desktop* child);
    void resize(Direction direction, long offset);
    void pushResize(Direction direction, long offset);
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
    RECT previousRect_; // used for undo
    RECT originalRect_; // used for undo from gaps

    Direction lastResizeDirection_ = -1;

    std::vector<Container*> getAllParentContainers(bool withRoot = true);
    RECT getOriginalRect();
    void printStructure(int depth = 0);
    void moveWindowToRect(RECT rect, int gap = 0);
    void resizeWindow(Direction direction, long offset);
    void pushResizeWindow(Direction direction, long offset); // pushing with negative offset is pulling
};

class Neighbours
{
public:
    std::vector<WindowData*> left_;
    std::vector<Container*> leftContainers_;
    std::vector<WindowData*> right_;
    std::vector<Container*> rightContainers_;
    std::vector<WindowData*> top_;
    std::vector<Container*> topContainers_;
    std::vector<WindowData*> bottom_;
    std::vector<Container*> bottomContainers_;
    std::vector<WindowData*> getNeighbours(Direction direction);
    std::vector<Container*> getContainerNeighbours(Direction direction);
    void addNeighbour(WindowData* window, int direction);
    void addNeighbour(Container* container, Direction direction);
    void clearNeighbours(Direction direction = -1);
    bool noNeighbours();
};
