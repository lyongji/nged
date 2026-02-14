# GraphItemPool

The `GraphItemPool` class manages the memory and lifecycle of graph items. It uses a free list to efficiently reuse memory slots.

## Header

`#include "nged/nged.h"`

## Class Definition

```cpp
class GraphItemPool
{
  // ...
};
```

## Key Responsibilities

-   **Memory Management**: Allocates and deallocates graph items.
-   **ID Mapping**: Maps `ItemID` (transient) to `GraphItemPtr`.
-   **UID Mapping**: Maps `UID` (persistent) to `ItemID`.

## Public Methods

### Item Management

-   `ItemID add(GraphItemPtr item)`: Adds an item to the pool and returns its ID.
-   `void release(ItemID id)`: Removes an item from the pool.
-   `GraphItemPtr get(ItemID id)`: Retrieves an item by its ID.
-   `GraphItemPtr get(UID const& uid)`: Retrieves an item by its UUID.

### Iteration

-   `template<class F> void foreach(F f) const`: Iterates over all active items in the pool.
-   `size_t count() const`: Returns the number of active items.
