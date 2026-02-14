# Link

The `Link` class represents a connection between an output port of one node and an input port of another node.

## Header

`#include "nged/nged.h"`

## Class Definition

```cpp
class Link : public GraphItem
{
  OutputConnection output_;
  InputConnection  input_;
  Vector<Vec2>     path_;
  // ...
};
```

## Key Responsibilities

-   **Connection Data**: Stores source (`InputConnection`) and destination (`OutputConnection`) information.
-   **Visuals**: Calculates and stores the path points for drawing the link wire.

## Public Methods

-   `OutputConnection const& output() const`: Returns the source connection info (Node ID + Port Index).
-   `InputConnection const& input() const`: Returns the destination connection info (Node ID + Port Index).
-   `auto const& path() const`: Returns the list of points defining the link's path.
-   `virtual void calculatePath()`: Recalculates the visual path of the link based on node positions.
-   `virtual void draw(Canvas* canvas, GraphItemState state) const`: Draws the link.
-   `virtual bool hitTest(Vec2 pt) const`: Checks if a point hits the link (for selection/interaction).

## Related Structs

### InputConnection
Refers to the source of a signal (Output port of a node).
```cpp
struct InputConnection {
  ItemID sourceItem;
  sint   sourcePort;
};
```

### OutputConnection
Refers to the destination of a signal (Input port of a node).
```cpp
struct OutputConnection {
  ItemID destItem;
  sint   destPort;
};
```
