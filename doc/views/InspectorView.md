# InspectorView

The `InspectorView` class provides a property editor interface for selected graph items.

## Header

`#include "nged/nged.h"`

## Class Definition

```cpp
class InspectorView : public GraphView
{
protected:
  std::weak_ptr<GraphView> linkedView_;
  HashSet<ItemID>          inspectingItems_;
  // ...
};
```

## Key Responsibilities

-   **Property Editing**: Displays and edits properties of selected items.
-   **View Linking**: Can be linked to a specific `NetworkView` to automatically inspect its selection.
-   **Locking**: Supports locking to a specific item or view.

## Public Methods

### Linkage

-   `void linkToView(GraphView* view)`: Links this inspector to another view.
-   `auto linkedView() const`: Returns the linked view.
-   `bool lockOnItem() const`: Returns true if locked to specific items.
-   `bool lockOnView() const`: Returns true if locked to a specific view.

### Inspection

-   `auto const& inspectingItems() const`: Returns the set of items being inspected.
-   `void setInspectingItems(HashSet<ItemID> const& ids)`: Sets the items to inspect.

### Event Handling

-   `void onViewEvent(GraphView* view, StringView eventType) override`: Responds to events from other views (e.g., selection change).
-   `void onGraphModified() override`: Updates the inspector when the graph changes.
