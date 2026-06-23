# NodeGraphDoc

`NodeGraphDoc` 是节点图编辑器中的文档对象。包含图数据，管理撤销/重做历史，处理文件 I/O。

## 头文件

`#include "nged/nged.h"`

## 核心职责

- **文档模型**：持有根图（`Graph`）和所有图元。
- **历史管理**：通过 `NodeGraphDocHistory` 提供撤销/重做。
- **文件 I/O**：保存和加载 `.ng` 格式文件。
- **脏标记**：追踪未保存的修改。

## 公开方法

### 文件操作

- `bool open(String path)`：从文件加载文档。
- `bool save()`：保存到 `savePath_`。
- `bool saveAs(String path)`：另存到指定路径并记住路径。
- `bool saveTo(String path)`：保存到指定路径但不记住路径。
- `StringView savePath() const`：返回当前保存路径。

### 图管理

- `GraphPtr root() const`：返回根图。
- `void makeRoot()`：初始化根图。
- `void notifyGraphModified(Graph* graph)`：通知图已修改。

### 状态

- `bool dirty() const`：返回是否有未保存修改。
- `bool readonly() const`：返回是否只读。
- `void undo()` / `void redo()`：撤销/重做。

### 图元管理

- `ItemID addItem(GraphItemPtr item)`：添加图元到文档池。
- `GraphItemPtr getItem(ItemID id)`：获取图元。
- `void removeItem(ItemID id)`：移除图元。

### 序列化

- `void setDeserializeInplace(bool)`：设置是否原地反序列化（UID 匹配）。
- `GraphItemPtr findItemByUID(UID const& uid)`：通过 UID 查找图元。
