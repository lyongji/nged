# GraphItem

`GraphItem` 是所有可存在于图中的对象（节点、连线、注释、路由等）的抽象基类。

## 头文件

`#include "nged/nged.h"`

## 类定义

```cpp
class GraphItem : public std::enable_shared_from_this<GraphItem>
{
protected:
  Graph* parent_;
  ItemID id_ = ID_None;
  UID    uid_ = {};
  AABB   aabb_ = {{0, 0}, {0, 0}}; // 局部轴对齐包围盒
  Vec2   pos_  = {0, 0};           // 位置
  // ...
};
```

## 核心职责

- **标识**：存储唯一运行时 ID（`ItemID`）和持久化 UUID（`UID`）。
- **空间**：管理位置（`pos_`）和包围盒（`aabb_`）。
- **序列化**：定义保存/加载状态的接口。
- **绘制与交互**：定义绘制和命中测试的接口。

## 公开方法

### 标识与层级

- `ItemID id() const`：返回临时运行时 ID。
- `UID uid() const`：返回持久化 UUID。
- `Graph* parent() const`：返回父图。

### 空间

- `Vec2 pos() const`：返回当前位置。
- `virtual bool moveTo(Vec2 to)`：将图元移动到新位置。
- `virtual bool canMove() const`：返回图元是否可移动。
- `AABB aabb() const`：返回世界空间包围盒。
- `virtual AABB localBound() const`：返回局部空间包围盒。
- `virtual int zOrder() const`：返回绘制顺序（z 轴）。

### 交互

- `virtual bool hitTest(Vec2 point) const`：检查点是否在图元内。
- `virtual bool hitTest(AABB box) const`：检查图元是否与矩形相交。
- `virtual void draw(Canvas*, GraphItemState state) const`：绘制图元。

### 序列化

- `virtual bool serialize(Json&) const`：将图元状态序列化为 JSON。
- `virtual bool deserialize(Json const&)`：从 JSON 反序列化图元状态。

### 类型转换

提供到派生类型的安全转换方法：
- `asNode()`、`asLink()`、`asRouter()`
- `asDyeable()`、`asResizable()`、`asGroupBox()`
