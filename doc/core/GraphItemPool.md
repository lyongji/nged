# GraphItemPool

`GraphItemPool` 管理图元的内存和生命周期。使用空闲链表高效重用内存槽位。

## 头文件

`#include "nged/nged.h"`

## 核心职责

- **内存管理**：分配和释放图元。
- **ID 映射**：将 `ItemID`（临时）映射到 `GraphItemPtr`。
- **UID 映射**：将 `UID`（持久化）映射到 `ItemID`。

## 公开方法

### 图元管理

- `ItemID add(GraphItemPtr item)`：添加图元到池中并返回其 ID。
- `void release(ItemID id)`：从池中移除图元。
- `GraphItemPtr get(ItemID id)`：通过 ID 获取图元。
- `GraphItemPtr get(UID const& uid)`：通过 UUID 获取图元。

### 遍历

- `template<class F> void foreach(F f) const`：遍历池中所有活跃图元。
- `size_t count() const`：返回活跃图元数量。
