# NodeGraphDoc

The `NodeGraphDoc` class represents a document in the node graph editor. It contains the graph data, manages undo/redo history, and handles file I/O.

## Header

`#include "nged/nged.h"`

## Class Definition

```cpp
class NodeGraphDoc : public std::enable_shared_from_this<NodeGraphDoc>
{
  // ...
};
```

## Key Responsibilities

-   **Data Container**: Holds the root `Graph` and the `GraphItemPool`.
-   **History Management**: Manages `NodeGraphDocHistory` for undo/redo functionality.
-   **Serialization**: Handles loading from and saving to files.
-   **State Tracking**: Tracks whether the document is dirty (modified) or read-only.

## Public Methods

### Lifecycle & I/O

-   `NodeGraphDoc(NodeFactoryPtr nodeFactory, GraphItemFactory const* itemFactory)`: Constructor.
-   `bool open(String path)`: Opens a document from the specified path.
-   `void close()`: Closes the document.
-   `bool save()`: Saves the document to the current save path.
-   `bool saveAs(String path)`: Saves the document to a new path and updates the save path.
-   `bool saveTo(String path)`: Saves the document to a path without updating the current save path.

### Item Management

-   `virtual ItemID addItem(GraphItemPtr item)`: Adds an item to the document's pool.
-   `virtual GraphItemPtr getItem(ItemID id)`: Retrieves an item by its ID.
-   `virtual void removeItem(ItemID id)`: Removes an item from the pool.
-   `virtual size_t numItems() const`: Returns the number of items in the document.
-   `GraphItemPtr findItemByUID(UID const& uid)`: Finds an item by its unique identifier (UUID).

### State & Properties

-   `StringView title() const`: Returns the document title.
-   `StringView savePath() const`: Returns the current file path.
-   `GraphPtr root() const`: Returns the root graph.
-   `bool dirty() const`: Returns true if the document has unsaved changes.
-   `void touch()`: Marks the document as modified.
-   `void untouch()`: Marks the document as unmodified.
-   `bool readonly() const`: Returns true if the document is read-only.
-   `void setReadonly(bool readonly)`: Sets the read-only status.

### History

-   `void undo()`: Undoes the last operation.
-   `void redo()`: Redoes the last undone operation.
-   `auto& history()`: Returns the history manager.
-   `auto editGroup(String message)`: Starts a grouped edit operation (transaction).

### Notifications

-   `void setModifiedNotifier(std::function<void(Graph*)> func)`: Sets a callback to be notified when the graph is modified.
-   `void notifyGraphModified(Graph* graph)`: Triggers the modification callback.
