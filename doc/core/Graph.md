# Graph

The `Graph` class represents a collection of connected items (nodes, links, etc.). It manages the structure of the node network.

## Header

`#include "nged/nged.h"`

## Class Definition

```cpp
class Graph : public std::enable_shared_from_this<Graph>
{
  // ...
};
```

## Key Responsibilities

-   **Structure Management**: Maintains the set of items and their connections (links).
-   **Traversal**: Provides methods to traverse the graph (top-down, bottom-up).
-   **Link Validation**: Checks if connections are allowed (e.g., preventing loops).
-   **Serialization**: Serializes and deserializes the graph structure.

## Public Methods

### Item & Link Access

-   `auto const& items() const`: Returns the set of item IDs in the graph.
-   `auto const& allLinks() const`: Returns all links in the graph.
-   `GraphItemPtr get(ItemID id) const`: Retrieves an item by ID.
-   `LinkPtr getLink(ItemID destItem, sint destPort)`: Gets the link connected to a specific input.

### Modification

-   `virtual ItemID add(GraphItemPtr item)`: Adds an item to the graph.
-   `virtual void remove(HashSet<ItemID> const& items)`: Removes a set of items.
-   `virtual bool move(HashSet<ItemID> const& items, Vec2 const& delta)`: Moves items by a vector.
-   `virtual LinkPtr setLink(ItemID sourceItem, sint sourcePort, ItemID destItem, sint destPort)`: Establishes a connection.
-   `virtual void removeLink(ItemID destItem, sint destPort)`: Removes a connection.
-   `virtual void clear()`: Clears all items from the graph.

### Traversal & Analysis

-   `bool traverse(GraphTraverseResult& result, ...)`: Generic traversal method.
-   `bool travelTopDown(GraphTraverseResult& result, ...)`: Traverses from sources to destinations (BFS).
-   `bool travelBottomUp(GraphTraverseResult& result, ...)`: Traverses from destinations to sources (BFS).
-   `bool checkLoopBottomUp(...)`: Checks for cycles in the graph.
-   `bool checkLinkIsAllowed(...)`: Validates if a link can be created.

### Pathfinding

-   `virtual Vector<Vec2> calculatePath(Vec2 start, Vec2 end, ...)`: Calculates a visual path for a link.

### Helper Methods

-   `NodePtr createNode(StringView type)`: Creates and adds a node of a given type.
-   `Vec2 pinPos(NodePin pin) const`: specific position of a pin on the graph canvas.
