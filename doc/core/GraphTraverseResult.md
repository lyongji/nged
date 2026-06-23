# GraphTraverseResult

`GraphTraverseResult` 存储图遍历操作（如拓扑排序、BFS）的结果。提供高效的方式按遍历顺序访问节点及其依赖关系。

## 头文件

`#include "nged/nged.h"`

## 核心职责

- **顺序保持**：按访问顺序存储节点。
- **依赖追踪**：高效存储节点间的输入/输出关系。
- **迭代**：提供迭代器和访问器遍历结果。

## 公开方法

### 访问

- `size_t size() const`：返回结果中节点数量。
- `Node* node(size_t index) const`：返回指定索引的节点。
- `Node* inputOf(size_t nodeIndex, int inputIndex) const`：返回指定节点的输入节点。
- `Node* outputOf(size_t nodeIndex, int outputIndex) const`：返回指定节点的输出节点。

### 迭代

- `Iterator begin() const`：返回起始迭代器。
- `Iterator end() const`：返回结束迭代器。
- `Accessor operator[](size_t index) const`：返回指定索引的访问器。
- `Accessor find(ItemID id) const`：通过 ID 查找节点。

## 内部类

### Accessor

辅助类，用于访问结果中的节点属性：

- `Node* node() const`：返回节点。
- `int inputCount() const`：返回输入数量。
- `int outputCount() const`：返回输出数量。
