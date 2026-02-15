# NetworkView

The `NetworkView` class is a specialized `GraphView` for visualizing and editing the node graph. It supports zooming, panning, and various interaction states.

## Header

`#include "nged/nged.h"`

## Class Definition

```cpp
class NetworkView : public GraphView
{
protected:
  std::unique_ptr<Canvas> canvas_ = {nullptr};
  // ...
};
```

## Key Responsibilities

-   **Graph Visualization**: Renders nodes, links, and other graph items using a `Canvas`.
-   **Interaction Management**: Manages a stack of `InteractionState` objects (e.g., selection, moving, connecting).
-   **Navigation**: Handles zooming and panning.
-   **Selection**: Manages the set of selected items.

## Public Methods

### Canvas & Rendering

-   `Canvas* canvas() const`: Returns the drawing canvas.
-   `void draw() override`: Renders the graph.
-   `void update(float dt) override`: Updates the view and interaction states.

### Selection

-   `auto const& selectedItems() const`: Returns the set of selected item IDs.
-   `void setSelectedItems(HashSet<ItemID> items)`: Sets the selection.
-   `ItemID hoveringItem() const`: Returns the ID of the item under the mouse.
-   `NodePin hoveringPin() const`: Returns the pin under the mouse.
-   `Node* solelySelectedNode() const`: Returns the single selected node, or nullptr.

### Navigation

-   `void zoomToSelected(float time, bool doScale, int easingOrder, Vec2 offset)`: Zooms to fit selected items.
-   `void navigate(NavDirection direction)`: Navigates in the specified direction.

### Interaction States

-   `void addState(InteractionStatePtr state)`: Adds an interaction state.
-   `InteractionStatePtr getState(StringView const& name) const`: Retrieves a state by name.
-   `template<class T> bool isActive() const`: Checks if a specific state type is active.
-   `template<class T> std::shared_ptr<T> getState() const`: Retrieves a state by type.

### Effects

-   `void addFadingText(String text, Vec2 pos, Color color, float duration)`: Adds a temporary text effect.

## Nested Classes

### InteractionState
Base class for custom interactions.
-   `virtual bool shouldEnter(NetworkView const*)`: Condition to start interaction.
-   `virtual void onEnter(NetworkView*)`: Called when starting.
-   `virtual void tick(NetworkView*, float dt)`: Called every frame.
-   `virtual void draw(NetworkView*)`: Called to draw custom UI.
-   `virtual void onExit(NetworkView*)`: Called when ending.
