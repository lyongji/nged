# NodeGraphEditor

`NodeGraphEditor` 是节点图编辑器应用的中心控制器。管理视图、文档、命令和工厂。

## 头文件

`#include "nged/nged.h"`

## 核心职责

- **视图管理**：创建、跟踪和销毁视图。
- **文档管理**：创建、打开和保存文档。
- **命令系统**：管理和分发命令。
- **工厂模式**：通过工厂创建节点、图元和视图。
- **事件系统**：提供 `GraphEventHub` 进行组件间通信。

## 公开方法

### 文档操作

- `DocPtr createNewDocAndDefaultViews()`：创建新文档并附加默认视图。
- `DocPtr openDoc(StringView path)`：打开文档。
- `bool saveDoc(DocPtr doc)`：保存文档。
- `bool saveDocAs(DocPtr doc, StringView path)`：另存文档。

### 视图操作

- `ViewPtr addView(DocPtr doc, String const& kind)`：为文档添加视图。
- `bool closeView(ViewPtr view, bool confirmIfNotSaved)`：关闭视图。
- `void broadcastViewEvent(GraphView* view, StringView eventType)`：广播视图事件。

### 图元操作

- `NodePtr createNode(Graph* graph, StringView type)`：创建节点。
- `ItemID addItem(Graph* graph, GraphItemPtr item)`：添加图元。
- `void removeItems(Graph* graph, HashSet<ItemID> const& items, ...)`：移除图元。
- `bool setLink(...)`：建立连线。
- `void removeLink(...)`：移除连线。

### 配置

- `void setNodeFactory(NodeFactoryPtr)`：设置节点工厂。
- `void setItemFactory(GraphItemFactoryPtr)`：设置图元工厂。
- `void setViewFactory(ViewFactoryPtr)`：设置视图工厂。
- `void setContextMenus(ContextMenuEntriesPtr)`：设置右键菜单项。

### 访问器

- `NodeFactoryPtr nodeFactory() const`：获取节点工厂。
- `CommandManager& commandManager()`：获取命令管理器。
- `GraphEventHub& events()`：获取事件总线。
