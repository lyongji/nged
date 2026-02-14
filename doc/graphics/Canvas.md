# Canvas

The `Canvas` class is an abstract base class that defines the drawing interface for the node graph editor. It abstracts the underlying rendering API (e.g., ImGui, OpenGL).

## Header

`#include "nged/nged.h"`

## Class Definition

```cpp
class Canvas
{
public:
  // ...
};
```

## Key Responsibilities

-   **Drawing Primitives**: Provides methods to draw lines, rectangles, circles, and polygons.
-   **Text Rendering**: Provides methods to draw text with specific styles.
-   **Coordinate System**: Manages the transformation between screen space and canvas space (pan/zoom).
-   **Layer Management**: Supports drawing on different layers (e.g., background, standard, foreground).

## Public Methods

### Coordinate Transformation

-   `Mat3 canvasToScreen() const`: Returns the transformation matrix from canvas to screen.
-   `Mat3 screenToCanvas() const`: Returns the transformation matrix from screen to canvas.
-   `void setViewPos(Vec2 pos)`: Sets the view position (pan).
-   `void setViewScale(float scale)`: Sets the view scale (zoom).

### Drawing Primitives

-   `virtual void drawLine(Vec2 a, Vec2 b, uint32_t color, float width)`: Draws a line.
-   `virtual void drawRect(Vec2 tl, Vec2 br, float radius, ShapeStyle style)`: Draws a rectangle.
-   `virtual void drawCircle(Vec2 center, float radius, int segments, ShapeStyle style)`: Draws a circle.
-   `virtual void drawPoly(Vec2 const* pts, sint num, bool closed, ShapeStyle style)`: Draws a polygon.

### Text

-   `virtual void drawText(Vec2 pos, StringView text, TextStyle const& style)`: Draws text at a position.
-   `virtual Vec2 measureTextSize(StringView text, TextStyle const& style)`: Measures the size of text.

### Layers

-   `void pushLayer(Layer layer)`: Pushes a new drawing layer.
-   `void popLayer()`: Pops the current layer.
-   `virtual void setCurrentLayer(Layer layer)`: Sets the active layer.

## Nested Types

### ShapeStyle
Defines style for shapes.
```cpp
struct ShapeStyle {
  bool     filled;
  uint32_t fillColor;
  float    strokeWidth;
  uint32_t strokeColor;
};
```

### TextStyle
Defines style for text.
```cpp
struct TextStyle {
  TextAlign         align;
  TextVerticalAlign valign;
  FontFamily        font;
  FontStyle         style;
  FontSize          size;
  uint32_t          color;
};
```
