# GraphItem

The `GraphItem` class is the abstract base class for all objects that can exist in a graph (nodes, links, comments, routers, etc.).

## Header

`#include "nged/nged.h"`

## Class Definition

```cpp
class GraphItem : public std::enable_shared_from_this<GraphItem>
{
protected:
  Graph* parent_;
  ItemID id_ = ID_None;
  UID    uid_ = {};
  AABB   aabb_ = {{0, 0}, {0, 0}}; // local aabb
  Vec2   pos_  = {0, 0};           // position
  // ...
};
```

## Key Responsibilities

-   **Identity**: Stores unique ID (`ItemID`) and persistent UUID (`UID`).
-   **Spatial**: Manages position (`pos_`) and bounding box (`aabb_`).
-   **Serialization**: Defines interface for saving/loading state.
-   **Drawing & Interaction**: Defines interface for drawing and hit testing.

## Public Methods

### Identity & Hierarchy

-   `ItemID id() const`: Returns the transient runtime ID.
-   `UID uid() const`: Returns the persistent UUID.
-   `Graph* parent() const`: Returns the parent graph.

### Spatial

-   `Vec2 pos() const`: Returns the current position.
-   `virtual bool moveTo(Vec2 to)`: Moves the item to a new position.
-   `virtual bool canMove() const`: Returns true if the item is movable.
-   `AABB aabb() const`: Returns the world-space bounding box.
-   `virtual AABB localBound() const`: Returns the local-space bounding box.
-   `virtual int zOrder() const`: Returns drawing order (z-index).

### Interaction

-   `virtual bool hitTest(Vec2 point) const`: Checks if a point is within the item.
-   `virtual bool hitTest(AABB box) const`: Checks if the item intersects with a box.
-   `virtual void draw(Canvas*, GraphItemState state) const`: Draws the item.

### Serialization

-   `virtual bool serialize(Json&) const`: Serializes item state to JSON.
-   `virtual bool deserialize(Json const&)`: Deserializes item state from JSON.

### Casting Helper
Provides safe casting methods to derived types:
-   `asNode()`
-   `asLink()`
-   `asRouter()`
-   `asDyeable()`
-   `asResizable()`
-   `asGroupBox()`
