# Canvas

`Canvas` 是定义节点图编辑器绘制接口的抽象基类。它抽象了底层渲染 API（如 ImGui/OpenGL）。

## 头文件

`#include "nged/nged.h"`

## 核心职责

- **绘制命令**：提供绘制线条、矩形、圆形、文字和图片的接口。
- **坐标变换**：管理画布→屏幕坐标变换（支持平移和缩放）。
- **图层系统**：支持分层绘制（5 层：Lower → Higher）。

## 坐标系统

- `viewPos_`：画布视口偏移（平移）。
- `viewScale_`：画布缩放比例。
- `canvasToScreen()`：将画布坐标转换为屏幕坐标。
- `screenToCanvas()`：将屏幕坐标转换为画布坐标。

## 图层

```cpp
enum class Layer : int {
  Lower = 0, Low, Standard, High, Higher, Count
};
```

- `pushLayer(Layer)`：推入新图层。
- `popLayer()`：弹出图层。

## 绘制方法

### 形状

- `drawRect(Vec2 topleft, Vec2 bottomright, float cornerradius, ShapeStyle)`：绘制矩形。
- `drawCircle(Vec2 center, float radius, int segments, ShapeStyle)`：绘制圆形。
- `drawPoly(Vec2 const* pts, sint count, bool closed, ShapeStyle)`：绘制多边形。
- `drawLine(Vec2 a, Vec2 b, uint32_t color, float width)`：绘制线段。

### 文字

- `drawText(Vec2 pos, StringView text, TextStyle)`：绘制文字（画布坐标）。
- `drawTextUntransformed(Vec2 pos, StringView text, TextStyle, float scale)`：绘制文字（屏幕坐标）。
- `measureTextSize(StringView text, TextStyle)`：测量文字尺寸。

### 图片

- `drawImage(ImagePtr image, Vec2 pmin, Vec2 pmax, ...)`：绘制图片。
- `static ImagePtr createImage(uint8_t const* data, int w, int h)`：创建图片对象。

## 样式

```cpp
struct ShapeStyle { bool filled; uint32_t fillColor; float strokeWidth; uint32_t strokeColor; };

struct TextStyle {
  TextAlign align; TextVerticalAlign valign;
  FontFamily font; FontStyle style; FontSize size;
  uint32_t color;
};
```
