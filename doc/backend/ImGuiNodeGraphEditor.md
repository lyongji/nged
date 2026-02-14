# ImGuiNodeGraphEditor

The `ImGuiNodeGraphEditor` class is a concrete implementation of `NodeGraphEditor` that uses the Dear ImGui library for rendering and input handling.

## Header

`#include "nged/nged_imgui.h"`

## Class Definition

```cpp
class ImGuiNodeGraphEditor : public NodeGraphEditor
{
  // ...
};
```

## Key Responsibilities

-   **ImGui Integration**: Implements the `draw()` method using ImGui calls.
-   **Layout Management**: Manages the layout of ImGui windows (docking, splitting).
-   **Clipboard**: Implements clipboard operations using ImGui's clipboard API.

## Public Methods

### Lifecycle & Rendering

-   `void draw() override`: Renders the editor using ImGui.
-   `void initCommands() override`: Initializes default commands.

### Layout

-   `void setDefaultLayoutDesc(String desc)`: Sets the default window layout description.
-   `auto const& defaultLayoutDesc() const`: Returns the default layout description.

### Clipboard

-   `void setClipboardText(StringView text) const override`: Sets text to the clipboard.
-   `String getClipboardText() const override`: Gets text from the clipboard.

### Factory

-   `DocPtr createNewDocAndDefaultViews() override`: Creates a new document with default ImGui views.
