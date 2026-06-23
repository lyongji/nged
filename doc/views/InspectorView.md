# InspectorView

`InspectorView` 为选中的图元提供属性编辑器界面。

## 头文件

`#include "nged/nged.h"`

## 类定义

```cpp
class InspectorView : public GraphView
{
protected:
  std::weak_ptr<GraphView> linkedView_;       // 关联的主视图
  HashSet<ItemID>          inspectingItems_;  // 正在检查的图元
  bool                     lockOnItem_ = false;
  bool                     lockOnView_ = false;
};
```

## 核心职责

- **关联视图**：通常链接到一个 `NetworkView`，跟随其选择变化。
- **属性编辑**：显示和编辑选中图元的属性（名称、颜色、位置等）。
- **锁定机制**：可以锁定到特定图元或特定视图，不受选择变化影响。

## 公开方法

- `GraphViewPtr linkedView() const`：返回关联的视图。
- `HashSet<ItemID> const& inspectingItems() const`：返回当前检查的图元集合。
- `void setInspectingItems(HashSet<ItemID> const& ids)`：设置检查目标。
- `void linkToView(GraphView* view)`：链接到一个视图。
- `bool lockOnItem() const`：是否锁定图元。
- `bool lockOnView() const`：是否锁定视图。

## 工作流程

1. 用户在 `NetworkView` 中选中图元。
2. 关联的 `InspectorView` 收到选择变化通知。
3. 显示选中图元的属性面板（名称、大小、颜色等）。
4. 用户可以直接在检查器中修改属性。
