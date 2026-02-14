# GraphTraverseResult

The `GraphTraverseResult` class stores the result of a graph traversal operation (e.g., topological sort, BFS). It provides an efficient way to access nodes in the traversed order and their dependencies.

## Header

`#include "nged/nged.h"`

## Class Definition

```cpp
class GraphTraverseResult
{
  // ...
};
```

## Key Responsibilities

-   **Order Preservation**: Stores nodes in the order they were visited.
-   **Dependency Tracking**: Efficiently stores input/output relationships between visited nodes.
-   **Iteration**: Provides iterators and accessors to traverse the result.

## Public Methods

### Access

-   `size_t size() const`: Returns the number of nodes in the result.
-   `Node* node(size_t index) const`: Returns the node at the given index.
-   `Node* inputOf(size_t nodeIndex, int inputIndex) const`: Returns the input node for a given node.
-   `Node* outputOf(size_t nodeIndex, int outputIndex) const`: Returns the output node for a given node.

### Iteration

-   `Iterator begin() const`: Returns an iterator to the beginning.
-   `Iterator end() const`: Returns an iterator to the end.
-   `Accessor operator[](size_t index) const`: Returns an accessor for the node at the given index.
-   `Accessor find(ItemID id) const`: Finds a node by its ID.

## Nested Classes

### Accessor
A helper class to access node properties within the result.
-   `Node* node() const`: Returns the node.
-   `int inputCount() const`: Returns the number of inputs.
-   `int outputCount() const`: Returns the number of outputs.
