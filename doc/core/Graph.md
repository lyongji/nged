# Graph

`Graph` 表示一组相互连接的图元（节点、连线等）集合。它管理节点网络的结构。

## 头文件

`#include "nged/nged.h"`

## 核心职责

- **结构管理**：维护图元集合及其连接关系（连线）。
- **图遍历**：提供自顶向下和自底向上的图遍历方法。
- **连线验证**：检查连接是否合法（如防止环路）。
- **序列化**：序列化和反序列化图结构。

## 公开方法

### 图元与连线访问

- `auto const& items() const`：返回图中所有图元 ID 集合。
- `auto const& allLinks() const`：返回图中所有连线。
- `GraphItemPtr get(ItemID id) const`：通过 ID 获取图元。
- `LinkPtr getLink(ItemID destItem, sint destPort)`：获取连接到指定输入的连线。

### 修改

- `virtual ItemID add(GraphItemPtr item)`：添加图元到图中。
- `virtual void remove(HashSet<ItemID> const& items)`：移除一组图元。
- `virtual bool move(HashSet<ItemID> const& items, Vec2 const& delta)`：按向量移动图元。
- `virtual LinkPtr setLink(ItemID sourceItem, sint sourcePort, ItemID destItem, sint destPort)`：建立连接。
- `virtual void removeLink(ItemID destItem, sint destPort)`：移除连接。
- `virtual void clear()`：清空图中所有图元。

### 遍历与分析

- `bool traverse(GraphTraverseResult& result, ...)`：通用遍历方法。
- `bool travelTopDown(GraphTraverseResult& result, ...)`：从源节点到目标节点遍历（BFS）。
- `bool travelBottomUp(GraphTraverseResult& result, ...)`：从目标节点回溯到源节点遍历（BFS）。
- `bool checkLoopBottomUp(...)`：检查图中是否存在环路。
- `bool checkLinkIsAllowed(...)`：验证是否可以创建连线。

### 路径计算

- `virtual Vector<Vec2> calculatePath(Vec2 start, Vec2 end, ...)`：计算连线的视觉路径。

### 辅助方法

- `NodePtr createNode(StringView type)`：创建并添加指定类型的节点。
- `Vec2 pinPos(NodePin pin) const`：获取引脚在图上的位置。
