# GraphView

The `GraphView` class is the base class for all UI views in the editor. A view presents data from a `NodeGraphDoc` to the user.

## Header

`#include "nged/nged.h"`

## Class Definition

```cpp
class GraphView : public std::enable_shared_from_this<GraphView>
{
protected:
  NodeGraphDocPtr  doc_ = nullptr;
  WeakGraphPtr     graph_;
  String           kind_      = "unknown";
  String           title_     = "untitled";
  // ...
};
```

## Key Responsibilities

-   **View State**: Manages open/closed state, focus, and hover status.
-   **Document Association**: Links to a `NodeGraphDoc`.
-   **Event Handling**: Provides virtual methods for handling events.
-   **Rendering Interface**: Defines the `draw` method for rendering.

## Public Methods

### Properties

-   `NodeGraphDocPtr doc() const`: Returns the associated document.
-   `GraphPtr graph() const`: Returns the graph currently being viewed (if any).
-   `StringView kind() const`: Returns the view kind (e.g., "network", "inspector").
-   `String const& title() const`: Returns the view title.
-   `void setTitle(String title)`: Sets the view title.
-   `bool isOpen() const`: Returns true if the view is open.
-   `void setOpen(bool open)`: Opens or closes the view.
-   `bool isFocused() const`: Returns true if the view has focus.
-   `bool isHovered() const`: Returns true if the mouse is hovering over the view.

### Lifecycle

-   `virtual void reset(NodeGraphDocPtr doc)`: Resets the view to a new document.
-   `virtual void reset(WeakGraphPtr graph)`: Resets the view to a new graph.
-   `virtual void update(float dt)`: Updates the view state.
-   `virtual void draw() = 0`: Draws the view content (pure virtual).

### Custom Events

-   `virtual void onViewEvent(GraphView* view, StringView eventType)`: Handles custom view events.
-   `virtual void please(StringView request)`: Handles generic requests.
-   `virtual void onDocModified() = 0`: Called when the document is modified.
-   `virtual void onGraphModified() = 0`: Called when the graph structure changes.
