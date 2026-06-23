# GraphView

`GraphView` 是编辑器中所有 UI 视图的基类。视图将 `NodeGraphDoc` 的数据呈现给用户。

## 头文件

`#include "nged/nged.h"`

## 核心职责

- **数据绑定**：关联到一个 `NodeGraphDoc` 和可选的一个 `Graph`。
- **生命周期**：管理视图的打开/关闭状态。
- **焦点管理**：追踪视图是否聚焦或悬停。
- **事件响应**：响应视图事件（焦点、选择等）。

## 公开方法

### 数据

- `NodeGraphDocPtr doc() const`：返回关联的文档。
- `GraphPtr graph() const`：返回当前显示的图。
- `virtual void reset(NodeGraphDocPtr doc)`：重置文档。
- `virtual void reset(WeakGraphPtr graph)`：切换到另一个图。

### 状态

- `bool isOpen() const` / `void setOpen(bool)`：视图开关状态。
- `bool isFocused() const` / `void setFocused(bool)`：焦点状态。
- `bool isHovered() const` / `void setHovered(bool)`：悬停状态。
- `size_t id() const`：视图唯一 ID。

### 更新与绘制

- `virtual void update(float dt)`：每帧更新。
- `virtual void draw() = 0`：绘制视图（纯虚函数）。
- `virtual void onDocModified()`：文档修改回调。
- `virtual void onGraphModified()`：图修改回调。

### 事件

- `virtual void onViewEvent(GraphView* view, StringView eventType)`：接收视图事件。
- `virtual void please(StringView request)`：响应其他视图的请求。

### 菜单

- `virtual bool hasMenu() const`：是否有菜单栏。
- `virtual void updateMenu()`：更新菜单栏内容。
