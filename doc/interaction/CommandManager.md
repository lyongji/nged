# CommandManager

`CommandManager` 管理应用程序命令的注册与执行，包括键盘快捷键和命令面板集成。

## 头文件

`#include "nged/nged.h"`

## 核心职责

- **命令注册**：注册全局或视图特定的命令。
- **快捷键处理**：检测快捷键并执行对应命令。
- **命令面板**：提供模糊搜索的命令面板（`Ctrl+P`）。
- **提示对话框**：支持需要用户输入参数的命令。

## 类

### Command

```cpp
class Command {
  String name_;           // 命令名称（如 "File/Save"）
  String description_;    // 命令描述
  String view_;           // 生效视图（"*" 表示全局）
  Shortcut shortcut_;     // 快捷键
  bool hiddenInMenu_;     // 是否隐藏（仅快捷键触发）
};
```

#### 公开方法

- `String const& name() const`：命令名称。
- `String const& description() const`：命令描述。
- `Shortcut const& shortcut() const`：快捷键。
- `virtual void onConfirm(GraphView* view) = 0`：执行命令。
- `virtual bool hasPrompt() const`：是否需要参数输入。
- `virtual void onOpenPrompt(GraphView* view)`：打开参数输入。
- `virtual bool onUpdatePrompt(GraphView* view)`：更新参数输入。

### SimpleCommand

`Command` 的便捷子类，使用 lambda 实现命令逻辑：

```cpp
mgr.add(new CommandManager::SimpleCommand{
  "File/Save",                          // 名称
  "保存当前文档",                        // 描述
  [](GraphView* view, StringView args) { // 执行逻辑
    view->editor()->saveDoc(view->doc());
  },
  Shortcut{'S', ModKey::CTRL},          // Ctrl+S
  "*",                                   // 全局可用
});
```

## Shortcut

```cpp
struct Shortcut {
  uint8_t key;   // 按键（ASCII 字符或特殊键码，如 0xF2=F2）
  ModKey  mod;   // 修饰键（Ctrl/Shift/Alt/Super 的组合）
};
```

- `Shortcut::parse("Ctrl+Z")`：从字符串解析快捷键。
- `Shortcut::describe(shortcut)`：格式化快捷键为可读字符串。

## CommandManager 公开方法

- `Command& add(CommandPtr cmd)`：注册命令。
- `bool remove(String const& name)`：移除命令。
- `void checkShortcut(GraphView* view)`：检测当前视图的快捷键。
- `void openPalette()`：打开命令面板。
- `void prompt(CommandPtr cmd, GraphView* view)`：打开命令的提示输入。
- `Vector<CommandPtr> const& commands() const`：获取所有注册命令。
