# Node

`Node` 是图中所有节点的基类。它代表一个具有输入和输出的功能单元。

## 头文件

`#include "nged/nged.h"`

## 核心职责

- **数据模型**：管理节点类型、名称、颜色和标志位。
- **引脚定义**：定义输入和输出引脚的数量和位置。
- **类型检查**：验证连接的兼容性（通过 `TypedNode` 子类）。
- **自定义绘制**：允许子类自定义节点外观。

## 公开方法

### 数据模型

- `String const& type() const`：返回节点类型标识。
- `String const& name() const`：返回节点名称。
- `virtual bool rename(String const& desired, String& accepted)`：重命名节点。
- `virtual uint64_t flags() const`：返回节点标志位（如 Bypass）。

### 引脚

- `virtual sint numMaxInputs() const`：返回最大输入数（<0 表示不限制）。
- `virtual sint numFixedInputs() const`：固定输入引脚数量。
- `virtual sint numOutputs() const`：返回输出引脚数。
- `virtual bool acceptInput(sint port, Node const* src, sint srcPort) const`：检查是否接受连接。
- `Vec2 inputPinPos(sint i) const`：返回输入引脚位置。
- `Vec2 outputPinPos(sint i) const`：返回输出引脚位置。

### 绘制

- `virtual void draw(Canvas* canvas, GraphItemState state) const`：绘制节点。
- `virtual bool getIcon(IconType& type, StringView& content) const`：获取节点图标。
- `virtual bool getNodeDescription(String& desc) const`：获取节点描述。

### 类型转换

- `virtual Graph const* asGraph() const`：如果节点包含子图则返回子图。
- `virtual TypedNode const* asTypedNode() const`：返回类型化节点接口。
