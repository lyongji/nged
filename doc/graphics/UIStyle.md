# UIStyle

The `UIStyle` struct defines the visual appearance of the node graph editor. It contains colors, sizes, and other style properties.

## Header

`#include "nged/style.h"`

## Class Definition

```cpp
struct UIStyle
{
  uint32_t windowBackgroundColor    = 0x333333ff;
  uint32_t nodeDefaultColor         = 0xddddddff;
  float    nodeStrokeWidth          = 0.f;
  // ...
  static UIStyle& instance();
  void save();
  void load();
};
```

## Key Responsibilities

-   **Theme Configuration**: Stores colors for nodes, links, selection, etc.
-   **Layout Metrics**: Stores sizes for fonts, margins, and radii.
-   **Persistence**: Can be saved to and loaded from a file.

## Public Members

### Colors

-   `windowBackgroundColor`: Background color of the editor window.
-   `nodeDefaultColor`: Default color for nodes.
-   `nodeStrokeColor`: Stroke color for nodes.
-   `linkDefaultColor`: Default color for links.
-   `linkSelectedColor`: Color for selected links.
-   `selectionBoxBackground`: Background color of the selection box.
-   `commentColor`: Text color for comments.
-   `groupBoxBackground`: Background color for group boxes.

### Metrics

-   `nodeStrokeWidth`: Width of the node border.
-   `nodePinRadius`: Radius of input/output pins.
-   `linkStrokeWidth`: Width of link lines.
-   `routerRadius`: Radius of router points.
-   `bigFontSize`, `normalFontSize`, `smallFontSize`: Font sizes.

## Methods

-   `static UIStyle& instance()`: Returns the singleton instance.
-   `void save()`: Saves the current style to disk.
-   `void load()`: Loads the style from disk.
