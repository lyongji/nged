# NodeGraphEditResponser

The `NodeGraphEditResponser` class acts as a hook or listener for graph editing events. It allows for custom validation and side effects during graph manipulation.

## Header

`#include "nged/nged.h"`

## Class Definition

```cpp
class NodeGraphEditResponser
{
public:
  virtual ~NodeGraphEditResponser() {}
  // ...
};
```

## Key Responsibilities

-   **Validation**: Can prevent actions by returning `false` (e.g., `beforeLinkSet`).
-   **Notification**: Receives callbacks after actions occur (e.g., `afterItemAdded`).
-   **Interaction Hooks**: Responds to mouse clicks and selection changes.

## Public Methods

### Validation Hooks (Return `false` to cancel)

-   `virtual bool beforeItemAdded(Graph* graph, GraphItem* item, GraphItem** replacement)`: Called before adding an item. Can modify the item to be added.
-   `virtual bool beforeItemRemoved(Graph* graph, GraphItem* item)`: Called before removing an item.
-   `virtual bool beforeNodeRenamed(Graph* graph, Node* node)`: Called before renaming a node.
-   `virtual bool beforeViewRemoved(GraphView* view)`: Called before removing a view.
-   `virtual bool beforeLinkSet(Graph* graph, InputConnection src, OutputConnection dst)`: Called before creating a link.

### Notification Hooks

-   `virtual void afterItemAdded(Graph* graph, GraphItem* item)`: Called after adding an item.
-   `virtual void afterNodeRenamed(Graph* graph, Node* node)`: Called after renaming a node.
-   `virtual void afterViewRemoved(GraphView* view)`: Called after removing a view.
-   `virtual void onLinkSet(Link* link)`: Called after a link is created.
-   `virtual void onLinkRemoved(Link* link)`: Called after a link is removed.
-   `virtual void onItemAdded(GraphItem* item)`: Called when an item is added.
-   `virtual void onItemMoved(GraphItem* item)`: Called when an item is moved.
-   `virtual void onItemModified(GraphItem* item)`: Called when an item is modified.
-   `virtual void onItemRemoved(GraphItem* item)`: Called when an item is removed.

### Interaction Hooks

-   `virtual void onItemClicked(NetworkView* view, GraphItem* item, int button)`: Mouse click handler.
-   `virtual void onItemDoubleClicked(NetworkView* view, GraphItem* item, int button)`: Double click handler.
-   `virtual void onItemHovered(NetworkView* view, GraphItem* item)`: Hover handler.
-   `virtual void onItemSelected(NetworkView* view, GraphItem* item)`: Selection handler.
-   `virtual void onItemDeselected(NetworkView* view, GraphItem* item)`: Deselection handler.
-   `virtual void onSelectionChanged(NetworkView* view)`: Called when selection changes.

### View Lifecycle

-   `virtual void beforeViewUpdate(GraphView* view)`: Called before view update.
-   `virtual void afterViewUpdate(GraphView* view)`: Called after view update.
-   `virtual void beforeViewDraw(GraphView* view)`: Called before view draw.
-   `virtual void afterViewDraw(GraphView* view)`: Called after view draw.
