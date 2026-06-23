# TypeSystem

`TypeSystem` 管理类型化节点和连接的类型定义与转换规则。确保只有兼容的端口能够被连接。

## 头文件

`#include "nged/nged.h"`

## 核心职责

- **类型注册**：注册全局唯一的类型标识。
- **继承关系**：定义类型之间的基础类型关系。
- **可转换性**：定义类型之间是否可转换。
- **颜色提示**：为每种类型关联颜色，用于连线和引脚的可视化提示。

## 使用

```cpp
auto& ts = TypeSystem::instance();

// 注册类型
ts.registerType("float");
ts.registerType("int", "float");              // int 的基础类型是 float
ts.registerType("vec3", "", Color{255,0,0});  // 带颜色提示

// 设置可转换性
ts.setConvertable("int", "float", true);      // int 可转换为 float
ts.setConvertable("float", "int", false);     // float 不可转换为 int
```

`TypedNode` 自动使用 `TypeSystem` 进行连接验证：

```cpp
class MyTypedNode : public TypedNode {
  MyTypedNode(Graph* parent)
    : TypedNode(parent, "my_node", "My Node", {"float"}, {"vec3"})
    //                                                ↑输入类型   ↑输出类型
  {}
};
```

## 公开方法

- `static TypeSystem& instance()`：获取单例。
- `TypeIndex registerType(StringView type, StringView baseType, Color hint)`：注册类型并返回索引。
- `void setConvertable(StringView from, StringView to, bool)`：设置类型可转换性。
- `bool isConvertable(StringView from, StringView to) const`：检查是否可转换。
- `bool isType(StringView type) const`：检查类型是否已注册。
- `TypeIndex typeIndex(StringView type) const`：获取类型索引。
- `StringView typeName(TypeIndex index) const`：获取类型名称。
- `Optional<Color> colorHint(TypeIndex index) const`：获取类型颜色提示。
