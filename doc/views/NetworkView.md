# NetworkView

`NetworkView` 是专门用于可视化和编辑节点图的 `GraphView`。支持缩放、平移和多种交互状态。

## 头文件

`#include "nged/nged.h"`

## 核心职责

- **图渲染**：通过 `Canvas` 绘制节点、连线和注释。
- **交互状态**：管理一组按优先级排序的交互状态（`InteractionState`），处理用户输入。
- **选择管理**：追踪选中和悬停的图元。
- **视觉效果**：支持动画和临时效果（如渐隐文字）。

## 公开方法

### 画布

- `Canvas* canvas() const`：返回画布对象。
- `bool canvasIsFocused() const`：画布是否聚焦。

### 选择

- `HashSet<ItemID> const& selectedItems() const`：返回选中图元集合。
- `void setSelectedItems(HashSet<ItemID> items)`：设置选中集合。
- `Node* solelySelectedNode() const`：返回唯一选中的节点（多选则返回 nullptr）。

### 交互状态

- `template<class T> void registerInteraction()`：注册交互状态。
- `bool isActive(StringView const& name) const`：检查状态是否激活。
- `InteractionStatePtr getState(StringView const& name) const`：获取交互状态。

### 导航

- `void zoomToSelected(float time, ...)`：缩放到选中图元。
- `void navigate(NavDirection direction)`：方向键导航到相邻节点。

### 剪贴板

- `bool copyTo(Json&)`：复制选中图元到 JSON。
- `bool pasteFrom(Json const&)`：从 JSON 粘贴图元。

## 交互状态栈

`NetworkView` 使用优先级排序的状态栈处理用户输入：

| 状态 | 优先级 | 功能 |
|------|--------|------|
| `HandleView` | 0 | 平移/缩放 |
| `AnimationState` | 10 | 平滑动画 |
| `MoveState` | 10 | 拖拽移动 |
| `SelectionState` | 50 | 框选/点选 |
| `LinkState` | 50 | 拖拽连线 |
| `CreateNodeState` | 50 | 搜索创建节点 |
| `CutLinkState` | 50 | 切断连线 |
| `HandleShortcut` | 100 | 快捷键处理 |

状态按优先级排序执行，高优先级状态可以阻止低优先级状态的更新。
