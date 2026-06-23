# Link

`Link` 表示一个节点的输出端口与另一个节点的输入端口之间的连线。

## 头文件

`#include "nged/nged.h"`

## 类定义

```cpp
class Link : public GraphItem
{
  OutputConnection output_;  // 源连接信息
  InputConnection  input_;   // 目标连接信息
  Vector<Vec2>     path_;    // 连线路径点
};
```

## 核心职责

- **连接数据**：存储源（`InputConnection`）和目标（`OutputConnection`）信息。
- **视觉呈现**：计算并存储用于绘制连线的路径点。

## 公开方法

- `OutputConnection const& output() const`：返回源连接信息（节点 ID + 端口索引）。
- `InputConnection const& input() const`：返回目标连接信息（节点 ID + 端口索引）。
- `auto const& path() const`：返回连线路径点列表。
- `virtual void calculatePath()`：根据节点位置重新计算连线视觉路径。
- `virtual void draw(Canvas* canvas, GraphItemState state) const`：绘制连线。
- `virtual bool hitTest(Vec2 pt) const`：检查点是否命中连线（用于选择和交互）。

## 相关结构体

### InputConnection

引用信号的来源（节点的输出端口）：

```cpp
struct InputConnection {
  ItemID sourceItem;  // 源节点 ID
  sint   sourcePort;  // 源端口索引
};
```

### OutputConnection

引用信号的目标（节点的输入端口）：

```cpp
struct OutputConnection {
  ItemID destItem;  // 目标节点 ID
  sint   destPort;  // 目标端口索引
};
```
